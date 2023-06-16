/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

import * as leo from '@brave/leo/tokens/css'

export const root = styled.div`
  background: linear-gradient(125.83deg, #392DD1 0%, #A91B78 99.09%);
  border-radius: 14px;
  font-family: var(--brave-font-heading);
  color: var(--brave-palette-white);
`

export const statusPanel = styled.div`
  padding: 16px 24px 9px 14px;
  display: flex;
`

export const statusIndicator = styled.div`
  flex: 1 0 auto;
`

export const earnings = styled.div`
  font-size: 11px;
  line-height: 16px;
`

export const earningsHeader = styled.div`
  display: flex;
  gap: 4px;
`

export const earningsHeaderTitle = styled.div`
  opacity: 0.65;
`

export const earningsInfo = styled.div`
  position: relative;

  leo-icon {
    opacity: 0.65;
    --leo-icon-size: 14px;
  }

  .tooltip {
    position: absolute;
    z-index: 1;
    top: 100%;
    right: -13px;
    width: 207px;
    padding-top: 8px;
    visibility: hidden;
    transition: visibility 0s linear 300ms;
  }

  &:hover .tooltip {
    visibility: initial;
  }
`

export const earningsTooltip = styled.div`
  position: relative;
  padding: 16px;
  background: ${leo.color.white};
  box-shadow: 0px 0px 24px rgba(99, 105, 110, 0.36);
  border-radius: 8px;
  color: ${leo.color.light.text.primary};
  font-size: 12px;
  line-height: 18px;
  font-weight: 400;

  &:before {
    content: '';
    position: absolute;
    top: -3px;
    right: 13px;
    background: inherit;
    height: 15px;
    width: 15px;
    transform: rotate(45deg);
  }
`

export const manageAds = styled.div`
  margin-top: 14px;

  button {
    color: ${leo.color.light.text.interactive};
    font-weight: 600;
    font-size: 12px;
    line-height: 16px;
    padding: 0;
    border: none;
    background: transparent;
    cursor: pointer;
    display: flex;
    align-items: center;
    gap: 5px;
  }

  .icon {
    height: 9px;
    width: auto;
    color: ${leo.color.light.icon.interactive};
  }
`

export const earningsDisplay = styled.div`
  margin-top: 4px;
  display: flex;
  align-items: center;
  gap: 4px;
`

export const earningsMonth = styled.div`
  padding: 2px 4px;
  background: rgba(255, 255, 255, 0.15);
  border-radius: 4px;
  opacity: 0.65;
`

export const earningsAmount = styled.div`
  font-size: 12px;
  line-height: 18px;

  .amount {
    opacity: 1;
  }

  .currency {
    opacity: 0.66;
  }
`

export const rewardsBalance = styled.div`
  margin: 0 32px;
`

export const disconnectedBalance = styled.div`
  margin: 6px 15px 0;
  width: 50%;
  padding: 12px;
  border-radius: 8px;
  background: linear-gradient(137.04deg, #346FE1 33.4%, #5844C3 82.8%);
  font-size: 12px;
  line-height: 18px;
  cursor: pointer;

  strong {
    font-weight: 600;
    color: #fff;
  }

  .icon {
    display: none;
    width: 20px;
    height: 20px;
    position: absolute;
    bottom: 1em;
    right: 1em;
  }
`

export const balanceHeader = styled.div`
  font-weight: 500;
  font-size: 14px;
  line-height: 24px;
  opacity: 0.65;
`

export const batAmount = styled.div`
  font-weight: 500;
  font-size: 32px;
  line-height: 48px;

  .currency {
    font-weight: 400;
    font-size: 14px;
    line-height: 24px;
  }
`

export const balanceSpinner = styled.div`
  .icon {
    height: 24px;
    vertical-align: middle;
    margin-right: 4px;
  }
`

export const loading = styled.span`
  font-size: 14px;
  line-height: 18px;
  vertical-align: middle;
`

export const exchangeAmount = styled.div`
  font-size: 12px;
  line-height: 14px;
  opacity: 0.66;
`

export const hiddenEarnings = styled.div`
  padding-left: 5px;
  display: flex;
  gap: 6px;

  a {
    color: #fff;
    text-decoration: none;
    font-weight: 600;
    font-size: 12px;
    line-height: 18px;
  }
`

export const hiddenEarningsValue = styled.span`
  opacity: 0.65;
`

export const viewStatement = styled.div`
  margin-top: 12px;
  text-align: center;

  button {
    font-weight: 500;
    font-size: 13px;
    line-height: 20px;
    padding: 0;
    border-radius: 48px;
    border: none;
    background: transparent;
    cursor: pointer;
  }

  .icon {
    width: 15px;
    height: auto;
    vertical-align: middle;
    margin-right: 6px;
    margin-bottom: 2px;
  }
`

export const summaryBox = styled.div`
  padding: 17px 17px 16px;
`

export const pendingBox = styled.div`
  margin-top: 17px;
  border-radius: 0 0 14px 14px;
  overflow: hidden;
  /* Moving this box down slightly ensures that the underlying background color
     will not show through the rounded corners. */
  position: relative;
  top: 2px;
`
