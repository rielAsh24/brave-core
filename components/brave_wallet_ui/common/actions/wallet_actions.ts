/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

 import { WalletActions } from '../slices/wallet.slice'

 // We must re-export actions here until we remove all imports of this file
 export const {
  accountsChanged,
  activeOriginChanged,
  addAccount,
  addFavoriteApp,
  addFilecoinAccount,
  addSitePermission,
  addUserAsset,
  updateUserAsset,
  addUserAssetError,
  approveERC20Allowance,
  approveTransaction,
  autoLockMinutesChanged,
  backedUp,
  cancelTransaction,
  chainChangedEvent,
  defaultBaseCryptocurrencyChanged,
  defaultBaseCurrencyChanged,
  defaultCurrenciesUpdated,
  defaultEthereumWalletChanged,
  defaultEthereumWalletUpdated,
  defaultSolanaWalletChanged,
  defaultSolanaWalletUpdated,
  expandWalletNetworks,
  getAllNetworks,
  getAllTokensList,
  getCoinMarkets,
  getOnRampCurrencies,
  hasIncorrectPassword,
  initialize,
  initialized,
  isEip1559Changed,
  keyringCreated,
  keyringReset,
  keyringRestored,
  lockWallet,
  locked,
  nativeAssetBalancesUpdated,
  newUnapprovedTxAdded,
  portfolioPriceHistoryUpdated,
  portfolioTimelineUpdated,
  pricesUpdated,
  queueNextTransaction,
  refreshAccountInfo,
  refreshBalancesAndPriceHistory,
  refreshBalancesAndPrices,
  refreshGasEstimates,
  rejectAllTransactions,
  rejectTransaction,
  removeFavoriteApp,
  removeSitePermission,
  removeUserAsset,
  retryTransaction,
  selectAccount,
  selectCurrency,
  selectNetwork,
  selectPortfolioTimeline,
  selectedAccountChanged,
  sendERC20Transfer,
  sendERC721TransferFrom,
  sendSPLTransfer,
  sendTransaction,
  setAccountTransactions,
  setAllNetworks,
  setAllTokensList,
  setAssetAutoDiscoveryCompleted,
  setCoinMarkets,
  setDefaultAccounts,
  setDefaultNetworks,
  setGasEstimates,
  setMetaMaskInstalled,
  setNetwork,
  setOnRampCurrencies,
  setPasswordAttempts,
  setSelectedAccount,
  setSelectedAccountFilterItem,
  setSelectedAssetFilterItem,
  setSelectedNetworkFilter,
  setSitePermissions,
  setSolFeeEstimates,
  setTransactionProviderError,
  setUserAssetVisible,
  setVisibleTokensInfo,
  speedupTransaction,
  tokenBalancesUpdated,
  transactionStatusChanged,
  unapprovedTxUpdated,
  unlockWallet,
  unlocked,
  updateUnapprovedTransactionGasFields,
  updateUnapprovedTransactionNonce,
  updateUnapprovedTransactionSpendAllowance,
  updateTokenPinStatus
 } = WalletActions
