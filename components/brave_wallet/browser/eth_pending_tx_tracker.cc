/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/eth_pending_tx_tracker.h"

#include <memory>
#include <utility>

#include "base/barrier_callback.h"
#include "base/containers/contains.h"
#include "base/logging.h"
#include "brave/components/brave_wallet/browser/eth_nonce_tracker.h"
#include "brave/components/brave_wallet/browser/eth_tx_meta.h"
#include "brave/components/brave_wallet/browser/json_rpc_service.h"
#include "brave/components/brave_wallet/browser/tx_meta.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_wallet {

EthPendingTxTracker::EthPendingTxTracker(EthTxStateManager* tx_state_manager,
                                         JsonRpcService* json_rpc_service,
                                         EthNonceTracker* nonce_tracker)
    : tx_state_manager_(tx_state_manager),
      json_rpc_service_(json_rpc_service),
      nonce_tracker_(nonce_tracker),
      weak_factory_(this) {}
EthPendingTxTracker::~EthPendingTxTracker() = default;

void EthPendingTxTracker::UpdatePendingTransactions(
    const absl::optional<std::string>& chain_id,
    UpdatePendingTxsCallback callback) {
  tx_state_manager_->GetTransactionsByStatus(
      chain_id, mojom::TransactionStatus::Submitted, absl::nullopt,
      base::BindOnce(
          &EthPendingTxTracker::ContinueUpdatePendingTransactionsSubmitted,
          weak_factory_.GetWeakPtr(), chain_id, std::move(callback)));
}

void EthPendingTxTracker::ContinueUpdatePendingTransactionsSubmitted(
    const absl::optional<std::string>& chain_id,
    UpdatePendingTxsCallback callback,
    std::vector<std::unique_ptr<TxMeta>> pending_transactions) {
  tx_state_manager_->GetTransactionsByStatus(
      chain_id, mojom::TransactionStatus::Signed, absl::nullopt,
      base::BindOnce(
          &EthPendingTxTracker::ContinueUpdatePendingTransactionsSigned,
          weak_factory_.GetWeakPtr(), std::move(pending_transactions),
          std::move(callback)));
}

void EthPendingTxTracker::ContinueUpdatePendingTransactionsSigned(
    std::vector<std::unique_ptr<TxMeta>> pending_transactions,
    UpdatePendingTxsCallback callback,
    std::vector<std::unique_ptr<TxMeta>> signed_transactions) {
  pending_transactions.insert(
      pending_transactions.end(),
      std::make_move_iterator(signed_transactions.begin()),
      std::make_move_iterator(signed_transactions.end()));

  const auto barrier_callback =
      base::BarrierCallback<absl::optional<std::string>>(
          pending_transactions.size(),
          base::BindOnce(
              &EthPendingTxTracker::FinalizeUpdatePendingTransactions,
              weak_factory_.GetWeakPtr(), std::move(callback)));
  for (auto& pending_transaction : pending_transactions) {
    tx_state_manager_->GetTransactionsByStatus(
        pending_transaction->chain_id(), mojom::TransactionStatus::Confirmed,
        absl::nullopt,
        base::BindOnce(&EthPendingTxTracker::ContinueUpdatePendingTransactions,
                       weak_factory_.GetWeakPtr(),
                       std::move(pending_transaction), barrier_callback));
  }
}

void EthPendingTxTracker::ContinueUpdatePendingTransactions(
    std::unique_ptr<TxMeta> pending_transaction,
    base::OnceCallback<void(absl::optional<std::string>)> barrier_callback,
    std::vector<std::unique_ptr<TxMeta>> confirmed_transactions) {
  CHECK(pending_transaction);
  if (IsNonceTaken(static_cast<const EthTxMeta&>(*pending_transaction),
                   confirmed_transactions)) {
    DropTransaction(pending_transaction.get());
    std::move(barrier_callback).Run(absl::nullopt);
  } else {
    const auto& pending_chain_id = pending_transaction->chain_id();
    std::string id = pending_transaction->id();
    json_rpc_service_->GetTransactionReceipt(
        pending_chain_id, pending_transaction->tx_hash(),
        base::BindOnce(&EthPendingTxTracker::OnGetTxReceipt,
                       weak_factory_.GetWeakPtr(), pending_chain_id,
                       std::move(id)));
    std::move(barrier_callback).Run(pending_chain_id);
  }
}

void EthPendingTxTracker::FinalizeUpdatePendingTransactions(
    UpdatePendingTxsCallback callback,
    const std::vector<absl::optional<std::string>>& pending_chain_ids) {
  std::set<std::string> result;
  for (const auto& pending_chain_id : pending_chain_ids) {
    if (pending_chain_id.has_value()) {
      result.emplace(*pending_chain_id);
    }
  }
  std::move(callback).Run(std::move(result));
}

void EthPendingTxTracker::Reset() {
  network_nonce_map_.clear();
  dropped_blocks_counter_.clear();
}

void EthPendingTxTracker::OnGetTxReceipt(const std::string& chain_id,
                                         std::string id,
                                         TransactionReceipt receipt,
                                         mojom::ProviderError error,
                                         const std::string& error_message) {
  if (error != mojom::ProviderError::kSuccess) {
    return;
  }

  tx_state_manager_->GetEthTx(
      chain_id, id,
      base::BindOnce(&EthPendingTxTracker::ContinueOnGetTxReceipt,
                     weak_factory_.GetWeakPtr(), std::move(receipt)));
}

void EthPendingTxTracker::ContinueOnGetTxReceipt(
    TransactionReceipt receipt,
    std::unique_ptr<EthTxMeta> meta) {
  if (!meta) {
    return;
  }
  if (receipt.status) {
    meta->set_tx_receipt(receipt);
    meta->set_status(mojom::TransactionStatus::Confirmed);
    meta->set_confirmed_time(base::Time::Now());
    tx_state_manager_->AddOrUpdateTx(*meta, base::DoNothing());
  } else if (ShouldTxDropped(*meta)) {
    DropTransaction(meta.get());
  }
}

void EthPendingTxTracker::OnGetNetworkNonce(const std::string& chain_id,
                                            const std::string& address,
                                            uint256_t result,
                                            mojom::ProviderError error,
                                            const std::string& error_message) {
  if (error != mojom::ProviderError::kSuccess) {
    return;
  }

  network_nonce_map_[address][chain_id] = result;
}

void EthPendingTxTracker::OnSendRawTransaction(
    const std::string& tx_hash,
    mojom::ProviderError error,
    const std::string& error_message) {}

bool EthPendingTxTracker::IsNonceTaken(
    const EthTxMeta& meta,
    const std::vector<std::unique_ptr<TxMeta>>& confirmed_transactions) {
  for (const auto& confirmed_transaction : confirmed_transactions) {
    auto* eth_confirmed_transaction =
        static_cast<EthTxMeta*>(confirmed_transaction.get());
    if (eth_confirmed_transaction->tx()->nonce() == meta.tx()->nonce() &&
        eth_confirmed_transaction->chain_id() == meta.chain_id() &&
        eth_confirmed_transaction->id() != meta.id()) {
      return true;
    }
  }
  return false;
}

bool EthPendingTxTracker::ShouldTxDropped(const EthTxMeta& meta) {
  const std::string& hex_address = meta.from();
  const std::string& chain_id = meta.chain_id();
  auto network_nonce_map_per_chain_id = network_nonce_map_.find(hex_address);
  if (network_nonce_map_per_chain_id == network_nonce_map_.end() ||
      !base::Contains(network_nonce_map_per_chain_id->second, chain_id)) {
    json_rpc_service_->GetEthTransactionCount(
        chain_id, hex_address,
        base::BindOnce(&EthPendingTxTracker::OnGetNetworkNonce,
                       weak_factory_.GetWeakPtr(), chain_id, hex_address));
  } else {
    uint256_t network_nonce = network_nonce_map_[hex_address][chain_id];
    network_nonce_map_per_chain_id->second.erase(chain_id);
    if (network_nonce_map_per_chain_id->second.empty()) {
      network_nonce_map_.erase(hex_address);
    }
    if (meta.tx()->nonce() < network_nonce) {
      return true;
    }
  }

  const std::string tx_hash = meta.tx_hash();
  if (!base::Contains(dropped_blocks_counter_, tx_hash)) {
    dropped_blocks_counter_[tx_hash] = 0;
  }
  if (dropped_blocks_counter_[tx_hash] >= 3) {
    dropped_blocks_counter_.erase(tx_hash);
    return true;
  }

  dropped_blocks_counter_[tx_hash] = dropped_blocks_counter_[tx_hash] + 1;

  return false;
}

void EthPendingTxTracker::DropTransaction(TxMeta* meta) {
  if (!meta) {
    return;
  }
  tx_state_manager_->DeleteTx(meta->chain_id(), meta->id(), base::DoNothing());
}

}  // namespace brave_wallet
