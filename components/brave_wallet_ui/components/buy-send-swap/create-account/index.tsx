// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

// Redux
import { useDispatch } from 'react-redux'
import { WalletActions } from '../../../common/actions'
import { PanelActions } from '../../../panel/actions'
import {
  useSafeWalletSelector,
  useUnsafeWalletSelector
} from '../../../common/hooks/use-safe-selector'
import { WalletSelectors } from '../../../common/selectors'
import {
  useGetSelectedChainQuery,
  useSetNetworkMutation
} from '../../../common/slices/api.slice'

// Types
import { BraveWallet } from '../../../constants/types'

// Components
import { NavButton } from '../../extension'
import { LockPanel } from '../../extension/lock-panel'

// Utils
import { getLocale } from '../../../../common/locale'
import { suggestNewAccountName } from '../../../utils/address-utils'
import { keyringIdForNewAccount } from '../../../utils/account-utils'

// Styled Components
import {
  StyledWrapper,
  Description,
  ButtonRow
} from './style'

export interface Props {
  isPanel?: boolean
  network?: BraveWallet.NetworkInfo
  prevNetwork?: BraveWallet.NetworkInfo | undefined
  onCancel?: () => void
}

export const CreateAccountTab = ({
  isPanel,
  prevNetwork,
  network,
  onCancel
}: Props) => {
  // redux
  const dispatch = useDispatch()
  const accounts = useUnsafeWalletSelector(WalletSelectors.accounts)
  const isWalletLocked = useSafeWalletSelector(WalletSelectors.isWalletLocked)

  // queries
  const { data: selectedNetwork } = useGetSelectedChainQuery()
  const [setNetwork] = useSetNetworkMutation()

  // state
  const [showUnlock, setShowUnlock] = React.useState<boolean>(false)

  // memos
  const accountNetwork = React.useMemo(() => {
    return network || selectedNetwork
  }, [network, selectedNetwork])

  const suggestedAccountName = React.useMemo((): string => {
    return accountNetwork
      ? suggestNewAccountName(accounts, accountNetwork)
      : ''
  }, [accounts, accountNetwork])

  // methods
  const onCancelCreateAccount = React.useCallback(async () => {
    if (onCancel) {
      return onCancel()
    }
    if (prevNetwork) {
      await setNetwork({
        chainId: prevNetwork.chainId,
        coin: prevNetwork.coin
      })
    }

    // prevNetwork can become undefined if the 
    // user closes and then reopens
    // the wallet, we need to have a fallback network
    // when the user clicks No or else the user will get
    // stuck on the createAccount screen.
    if (!prevNetwork) {
      await setNetwork({
        chainId: BraveWallet.MAINNET_CHAIN_ID,
        coin: BraveWallet.CoinType.ETH
      })
    }

    if (isPanel) {
      dispatch(PanelActions.navigateTo('main'))
    }
  }, [onCancel, prevNetwork, isPanel, setNetwork])

  const onCreateAccount = React.useCallback(() => {
    // unlock needed to create accounts
    if (isWalletLocked && !showUnlock) {
      return setShowUnlock(true)
    }

    // Do nothing if accountNetwork is undefined
    if (!accountNetwork) {
      return
    }

    dispatch(
      WalletActions.addAccount({
        coin: accountNetwork.coin,
        keyringId: keyringIdForNewAccount(
          accountNetwork.coin,
          accountNetwork.chainId
        ),
        accountName: suggestedAccountName
      })
    )

    if (isPanel) {
      dispatch(PanelActions.navigateTo('main'))
    }
  }, [
    isWalletLocked,
    showUnlock,
    accountNetwork,
    suggestedAccountName,
    isPanel
  ])

  const handleUnlockAttempt = React.useCallback((password: string): void => {
    dispatch(WalletActions.unlockWallet({ password }))
    onCreateAccount()
  }, [onCreateAccount])

  // effects
  React.useEffect(() => {
    // hide unlock screen on unlock success
    if (!isWalletLocked && showUnlock) {
      setShowUnlock(false)
    }
  }, [isWalletLocked, showUnlock])

  // render
  if (isWalletLocked && showUnlock) {
    return <StyledWrapper>
      <Description style={{ fontSize: 16 }}>
        {getLocale('braveWalletUnlockNeededToCreateAccount')}
      </Description>
      <LockPanel
        hideBackground
        onSubmit={handleUnlockAttempt}
      />
    </StyledWrapper>
  }

  return (
    <StyledWrapper>
      <Description>
        {accountNetwork
          ? getLocale('braveWalletCreateAccountDescription').replace('$1', accountNetwork.symbolName)
          : ''
        }
      </Description>

      <ButtonRow>
        <NavButton
          buttonType='secondary'
          onSubmit={onCancelCreateAccount}
          text={getLocale('braveWalletCreateAccountNo')}
        />
        <NavButton
          buttonType='primary'
          onSubmit={onCreateAccount}
          text={getLocale('braveWalletCreateAccountYes')}
        />
      </ButtonRow>
    </StyledWrapper>
  )
}

export default CreateAccountTab
