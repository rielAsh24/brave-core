/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
/* global window */

import { createReducer } from 'redux-act'
import { PanelState, SwapResponse } from '../../constants/types'
import * as PanelActions from '../actions/wallet_panel_actions'
import { ShowConnectToSitePayload, EthereumChainPayload } from '../constants/action_types'

const defaultState: PanelState = {
  // TODO(bbondy): isConnected, connectedSiteOrigin, and accounts is just test
  // data to start with until the keyring controller is ready.
  isConnected: false,
  hasInitialized: false,
  connectedSiteOrigin: 'https://app.uniswap.org',
  selectedPanel: 'main',
  panelTitle: '',
  tabId: -1,
  connectingAccounts: [],
  networkPayload: {
    chainId: '0x1', chainName: 'Ethereum Mainnet',
    rpcUrls: ['https://mainnet-infura.brave.com/'], blockExplorerUrls: [],
    iconUrls: [], symbol: 'ETH', symbolName: 'Ethereum', decimals: 18
  },
  swapQuote: undefined
}

const reducer = createReducer<PanelState>({}, defaultState)

reducer.on(PanelActions.navigateTo, (state: any, selectedPanel: string) => {
  let panelTitle = selectedPanel
  if (selectedPanel === 'networks') {
    // TODO(bbondy): This should be hooked up a localization label
    panelTitle = 'Select Network'
  }

  return {
    ...state,
    selectedPanel,
    panelTitle
  }
})

reducer.on(PanelActions.showConnectToSite, (state: any, payload: ShowConnectToSitePayload) => {
  return {
    ...state,
    tabId: payload.tabId,
    connectedSiteOrigin: payload.origin,
    connectingAccounts: payload.accounts
  }
})

reducer.on(PanelActions.addEthereumChain, (state: any, networkPayload: EthereumChainPayload) => {
  return {
    ...state,
    networkPayload: networkPayload.chain
  }
})

reducer.on(PanelActions.setPanelSwapQuote, (state: any, payload: SwapResponse) => {
  return {
    ...state,
    swapQuote: payload
  }
})

export default reducer
