/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { createReducer, createAction } from 'redux-act'
import { WalletAccountType, AccountModalTypes } from '../../constants/types'

export interface AccountsTabState {
  selectedAccount: WalletAccountType | undefined
  showAccountModal: boolean
  accountModalType: AccountModalTypes
}

const defaultState: AccountsTabState = {
  selectedAccount: undefined,
  showAccountModal: false,
  accountModalType: 'deposit'
}

export const AccountsTabActions = {
  setSelectedAccount: createAction<WalletAccountType | undefined>('setSelectedAccount'),
  setShowAccountModal: createAction<boolean>('setShowAccountModal'),
  setAccountModalType: createAction<AccountModalTypes>('setAccountModalType')
}

export const createAccountsTabReducer = (initialState: AccountsTabState) => {
  const reducer = createReducer<AccountsTabState>({}, initialState)

  reducer.on(AccountsTabActions.setSelectedAccount, (
    state: AccountsTabState,
    payload: WalletAccountType | undefined
  ): AccountsTabState => {
    return {
      ...state,
      selectedAccount: payload
    }
  })

  reducer.on(AccountsTabActions.setShowAccountModal, (
    state: AccountsTabState,
    payload: boolean
  ): AccountsTabState => {
    return {
      ...state,
      showAccountModal: payload
    }
  })

  reducer.on(AccountsTabActions.setAccountModalType, (
    state: AccountsTabState,
    payload: AccountModalTypes
  ): AccountsTabState => {
    return {
      ...state,
      accountModalType: payload
    }
  })

  return reducer
}

const accountsTabReducer = createAccountsTabReducer(defaultState)

export default accountsTabReducer
