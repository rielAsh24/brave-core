/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/keyring_controller.h"

#include <utility>

#include "base/base64.h"
#include "base/logging.h"
#include "brave/components/brave_wallet/browser/brave_wallet_constants.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/hd_keyring.h"
#include "brave/components/brave_wallet/browser/pref_names.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "components/sync_preferences/pref_service_syncable.h"
#include "crypto/random.h"

namespace brave_wallet {
namespace {
const size_t kSaltSize = 32;
const size_t kNonceSize = 12;
const char kDefaultKeyringId[] = "default";
const char kPasswordEncryptorSalt[] = "password_encryptor_salt";
const char kPasswordEncryptorNonce[] = "password_encryptor_nonce";
const char kEncryptedMnemonic[] = "encrypted_mnemonic";
const char kBackupComplete[] = "backup_complete";
const char kAccountMetas[] = "account_metas";
const char kAccountName[] = "account_name";

static base::span<const uint8_t> ToSpan(base::StringPiece sp) {
  return base::as_bytes(base::make_span(sp));
}
}  // namespace

KeyringController::KeyringController(PrefService* prefs) : prefs_(prefs) {
  DCHECK(prefs);
}

KeyringController::~KeyringController() {}

mojo::PendingRemote<mojom::KeyringController> KeyringController::MakeRemote() {
  mojo::PendingRemote<mojom::KeyringController> remote;
  receivers_.Add(this, remote.InitWithNewPipeAndPassReceiver());
  return remote;
}

void KeyringController::Bind(
    mojo::PendingReceiver<mojom::KeyringController> receiver) {
  receivers_.Add(this, std::move(receiver));
}

// static
void KeyringController::RegisterProfilePrefs(
    user_prefs::PrefRegistrySyncable* registry) {
  // TODO(bridiver) - move to brave/browser
  registry->RegisterIntegerPref(
      kBraveWalletWeb3Provider,
      static_cast<int>(brave_wallet::IsNativeWalletEnabled()
                           ? brave_wallet::Web3ProviderTypes::BRAVE_WALLET
                           : brave_wallet::Web3ProviderTypes::ASK));
  // TODO(bridiver) - move to brave/browser
  registry->RegisterBooleanPref(kShowWalletIconOnToolbar, true);

  // TODO(bridiver) - move to EthTxControllerFactory
  registry->RegisterDictionaryPref(kBraveWalletTransactions);

  registry->RegisterStringPref(kBraveWalletPasswordEncryptorSalt, "");
  registry->RegisterStringPref(kBraveWalletPasswordEncryptorNonce, "");
  registry->RegisterStringPref(kBraveWalletEncryptedMnemonic, "");
  registry->RegisterIntegerPref(kBraveWalletDefaultKeyringAccountNum, 0);
  registry->RegisterBooleanPref(kBraveWalletBackupComplete, false);
  registry->RegisterTimePref(kBraveWalletLastUnlockTime, base::Time());
  registry->RegisterListPref(kBraveWalletAccountNames);
  registry->RegisterDictionaryPref(kBraveWalletKeyrings);
}

// static
bool KeyringController::HasPrefForKeyring(PrefService* prefs,
                                          const std::string& key,
                                          const std::string& id) {
  return GetPrefForKeyring(prefs, key, id) != nullptr;
}

// static
const base::Value* KeyringController::GetPrefForKeyring(PrefService* prefs,
                                                        const std::string& key,
                                                        const std::string& id) {
  DCHECK(prefs);
  const base::DictionaryValue* keyrings_pref =
      prefs->GetDictionary(kBraveWalletKeyrings);
  if (!keyrings_pref)
    return nullptr;
  const base::Value* keyring_dict = keyrings_pref->FindKey(id);
  if (!keyring_dict)
    return nullptr;

  return keyring_dict->FindKey(key);
}

// static
void KeyringController::SetPrefForKeyring(PrefService* prefs,
                                          const std::string& key,
                                          base::Value value,
                                          const std::string& id) {
  DCHECK(prefs);
  DictionaryPrefUpdate update(prefs, kBraveWalletKeyrings);
  base::DictionaryValue* keyrings_pref = update.Get();

  if (!keyrings_pref->FindKey(id)) {
    keyrings_pref->SetKey(id, base::Value(base::Value::Type::DICTIONARY));
  }

  base::Value* keyring_dict = keyrings_pref->FindKey(id);
  if (!keyring_dict)
    return;

  keyring_dict->SetKey(key, std::move(value));
}

HDKeyring* KeyringController::CreateDefaultKeyring(
    const std::string& password) {
  if (!CreateEncryptorForKeyring(password, kDefaultKeyringId))
    return nullptr;

  const std::string mnemonic = GenerateMnemonic(16);
  if (!CreateDefaultKeyringInternal(mnemonic)) {
    return nullptr;
  }

  for (const auto& observer : observers_) {
    observer->KeyringCreated();
  }

  return default_keyring_.get();
}

HDKeyring* KeyringController::ResumeDefaultKeyring(
    const std::string& password) {
  if (!CreateEncryptorForKeyring(password, kDefaultKeyringId)) {
    return nullptr;
  }

  const std::string mnemonic = GetMnemonicForDefaultKeyringImpl();
  if (mnemonic.empty() || !CreateDefaultKeyringInternal(mnemonic)) {
    return nullptr;
  }
  size_t account_no = GetAccountMetasNumberForKeyring(kDefaultKeyringId);
  if (account_no)
    default_keyring_->AddAccounts(account_no);

  return default_keyring_.get();
}

HDKeyring* KeyringController::RestoreDefaultKeyring(
    const std::string& mnemonic,
    const std::string& password) {
  Reset();

  if (!CreateEncryptorForKeyring(password, kDefaultKeyringId))
    return nullptr;

  if (!CreateDefaultKeyringInternal(mnemonic)) {
    // When creation failed(ex. invalid mnemonic), clear the state
    Reset();
    return nullptr;
  }

  for (const auto& observer : observers_) {
    observer->KeyringRestored();
  }

  return default_keyring_.get();
}

void KeyringController::GetDefaultKeyringInfo(
    GetDefaultKeyringInfoCallback callback) {
  mojom::KeyringInfoPtr keyring = mojom::KeyringInfo::New();
  keyring->is_default_keyring_created = IsDefaultKeyringCreated();
  keyring->is_locked = IsLocked();
  bool backup_complete = false;
  const base::Value* value =
      GetPrefForKeyring(prefs_, kBackupComplete, kDefaultKeyringId);
  if (value)
    backup_complete = value->GetBool();
  keyring->is_backed_up = backup_complete;
  if (default_keyring_) {
    keyring->account_infos = GetAccountInfosForKeyring(kDefaultKeyringId);
  }
  std::move(callback).Run(std::move(keyring));
}

void KeyringController::GetMnemonicForDefaultKeyring(
    GetMnemonicForDefaultKeyringCallback callback) {
  std::move(callback).Run(GetMnemonicForDefaultKeyringImpl());
}

void KeyringController::CreateWallet(const std::string& password,
                                     CreateWalletCallback callback) {
  auto* keyring = CreateDefaultKeyring(password);
  if (keyring) {
    keyring->AddAccounts();
    size_t accounts_num = keyring->GetAccountsNumber();
    const std::string address = keyring->GetAddress(accounts_num - 1);
    // TODO(darkdh): use resource string
    SetAccountNameForKeyring(address, "Account 1", kDefaultKeyringId);
  }

  std::move(callback).Run(GetMnemonicForDefaultKeyringImpl());
}

void KeyringController::RestoreWallet(const std::string& mnemonic,
                                      const std::string& password,
                                      RestoreWalletCallback callback) {
  auto* keyring = RestoreDefaultKeyring(mnemonic, password);
  if (keyring) {
    keyring->AddAccounts();
    size_t accounts_num = keyring->GetAccountsNumber();
    const std::string address = keyring->GetAddress(accounts_num - 1);
    // TODO(darkdh): use resource string
    SetAccountNameForKeyring(address, "Account 1", kDefaultKeyringId);
  }
  // TODO(darkdh): add account discovery mechanism

  std::move(callback).Run(keyring);
}

const std::string KeyringController::GetMnemonicForDefaultKeyringImpl() {
  if (IsLocked()) {
    LOG(ERROR) << __func__ << ": Must Unlock controller first";
    return std::string();
  }
  DCHECK(encryptor_);
  std::vector<uint8_t> encrypted_mnemonic;

  if (!GetPrefInBytesForKeyring(kEncryptedMnemonic, &encrypted_mnemonic,
                                kDefaultKeyringId)) {
    return std::string();
  }
  std::vector<uint8_t> mnemonic;
  if (!encryptor_->Decrypt(encrypted_mnemonic,
                           GetOrCreateNonceForKeyring(kDefaultKeyringId),
                           &mnemonic)) {
    return std::string();
  }

  return std::string(mnemonic.begin(), mnemonic.end());
}

void KeyringController::AddAccount(const std::string& account_name,
                                   AddAccountCallback callback) {
  auto* keyring = GetDefaultKeyring();
  if (keyring) {
    keyring->AddAccounts();
    size_t accounts_num = keyring->GetAccountsNumber();
    const std::string address = keyring->GetAddress(accounts_num - 1);
    SetAccountNameForKeyring(address, account_name, kDefaultKeyringId);
  }

  for (const auto& observer : observers_) {
    observer->AccountsChanged();
  }
  std::move(callback).Run(keyring);
}

void KeyringController::IsWalletBackedUp(IsWalletBackedUpCallback callback) {
  bool backup_complete = false;
  const base::Value* value =
      GetPrefForKeyring(prefs_, kBackupComplete, kDefaultKeyringId);
  if (value)
    backup_complete = value->GetBool();
  std::move(callback).Run(backup_complete);
}

void KeyringController::NotifyWalletBackupComplete() {
  SetPrefForKeyring(prefs_, kBackupComplete, base::Value(true),
                    kDefaultKeyringId);
  for (const auto& observer : observers_) {
    observer->BackedUp();
  }
}

void KeyringController::SetAccountNameForKeyring(const std::string& address,
                                                 const std::string& name,
                                                 const std::string& id) {
  base::Value account_metas(base::Value::Type::DICTIONARY);
  const base::Value* value = GetPrefForKeyring(prefs_, kAccountMetas, id);
  if (value)
    account_metas = value->Clone();
  base::Value account_meta(base::Value::Type::DICTIONARY);
  account_meta.SetStringKey(kAccountName, name);
  account_metas.SetKey(address, std::move(account_meta));

  SetPrefForKeyring(prefs_, kAccountMetas, std::move(account_metas), id);
}

std::string KeyringController::GetAccountNameForKeyring(
    const std::string& address,
    const std::string& id) {
  const base::Value* account_metas =
      GetPrefForKeyring(prefs_, kAccountMetas, id);
  if (!account_metas)
    return std::string();

  const base::Value* name =
      account_metas->FindPath(address + "." + kAccountName);
  if (!name)
    return std::string();

  return name->GetString();
}

size_t KeyringController::GetAccountMetasNumberForKeyring(
    const std::string& id) {
  const base::Value* account_metas =
      GetPrefForKeyring(prefs_, kAccountMetas, id);
  if (!account_metas)
    return 0;

  return account_metas->DictSize();
}

std::vector<mojom::AccountInfoPtr> KeyringController::GetAccountInfosForKeyring(
    const std::string& id) {
  std::vector<mojom::AccountInfoPtr> result;
  if (!default_keyring_)
    return result;
  for (const std::string& address : default_keyring_->GetAccounts()) {
    mojom::AccountInfoPtr account_info = mojom::AccountInfo::New();
    account_info->address = address;
    account_info->name = GetAccountNameForKeyring(address, id);
    result.push_back(std::move(account_info));
  }
  return result;
}

HDKeyring* KeyringController::GetDefaultKeyring() {
  if (IsLocked()) {
    LOG(ERROR) << __func__ << ": Must Unlock controller first";
    return nullptr;
  }

  return default_keyring_.get();
}

bool KeyringController::IsLocked() const {
  return encryptor_ == nullptr;
}

void KeyringController::Lock() {
  if (IsLocked() || !default_keyring_)
    return;
  default_keyring_->ClearData();

  encryptor_.reset();
  for (const auto& observer : observers_) {
    observer->Locked();
  }
}

void KeyringController::Unlock(const std::string& password,
                               UnlockCallback callback) {
  if (!ResumeDefaultKeyring(password)) {
    encryptor_.reset();
    std::move(callback).Run(false);
    return;
  }

  UpdateLastUnlockPref(prefs_);
  for (const auto& observer : observers_) {
    observer->Unlocked();
  }
  std::move(callback).Run(true);
}

void KeyringController::IsLocked(IsLockedCallback callback) {
  std::move(callback).Run(IsLocked());
}

void KeyringController::Reset() {
  encryptor_.reset();

  default_keyring_.reset();

  prefs_->ClearPref(kBraveWalletKeyrings);
}

bool KeyringController::GetPrefInBytesForKeyring(const std::string& key,
                                                 std::vector<uint8_t>* bytes,
                                                 const std::string& id) const {
  if (!bytes)
    return false;

  const base::Value* value = GetPrefForKeyring(prefs_, key, id);
  if (!value)
    return false;

  const std::string* encoded = value->GetIfString();
  if (!encoded || encoded->empty())
    return false;

  std::string decoded;
  if (!base::Base64Decode(*encoded, &decoded)) {
    return false;
  }
  *bytes = std::vector<uint8_t>(decoded.begin(), decoded.end());
  return true;
}

void KeyringController::SetPrefInBytesForKeyring(
    const std::string& key,
    base::span<const uint8_t> bytes,
    const std::string& id) {
  const std::string encoded = base::Base64Encode(bytes);
  SetPrefForKeyring(prefs_, key, base::Value(encoded), id);
}

std::vector<uint8_t> KeyringController::GetOrCreateNonceForKeyring(
    const std::string& id) {
  std::vector<uint8_t> nonce(kNonceSize);
  if (!GetPrefInBytesForKeyring(kPasswordEncryptorNonce, &nonce, id)) {
    crypto::RandBytes(nonce);
    SetPrefInBytesForKeyring(kPasswordEncryptorNonce, nonce, id);
  }
  return nonce;
}

bool KeyringController::CreateEncryptorForKeyring(const std::string& password,
                                                  const std::string& id) {
  if (password.empty())
    return false;
  std::vector<uint8_t> salt(kSaltSize);
  if (!GetPrefInBytesForKeyring(kPasswordEncryptorSalt, &salt, id)) {
    crypto::RandBytes(salt);
    SetPrefInBytesForKeyring(kPasswordEncryptorSalt, salt, id);
  }
  encryptor_ = PasswordEncryptor::DeriveKeyFromPasswordUsingPbkdf2(
      password, salt, 100000, 256);
  return encryptor_ != nullptr;
}

bool KeyringController::CreateDefaultKeyringInternal(
    const std::string& mnemonic) {
  if (!encryptor_)
    return false;
  std::vector<uint8_t> encrypted_mnemonic;
  if (!encryptor_->Encrypt(ToSpan(mnemonic),
                           GetOrCreateNonceForKeyring(kDefaultKeyringId),
                           &encrypted_mnemonic)) {
    return false;
  }
  SetPrefInBytesForKeyring(kEncryptedMnemonic, encrypted_mnemonic,
                           kDefaultKeyringId);

  const std::unique_ptr<std::vector<uint8_t>> seed =
      MnemonicToSeed(mnemonic, "");
  if (!seed)
    return false;
  default_keyring_ = std::make_unique<HDKeyring>();
  default_keyring_->ConstructRootHDKey(*seed, "m/44'/60'/0'/0");
  UpdateLastUnlockPref(prefs_);

  return true;
}

bool KeyringController::IsDefaultKeyringCreated() {
  return HasPrefForKeyring(prefs_, kEncryptedMnemonic, kDefaultKeyringId);
}

void KeyringController::AddObserver(
    ::mojo::PendingRemote<mojom::KeyringControllerObserver> observer) {
  observers_.Add(std::move(observer));
}

}  // namespace brave_wallet
