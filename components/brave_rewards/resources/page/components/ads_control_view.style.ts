/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

import * as leo from '@brave/leo/tokens/css'

import * as mixins from '../../shared/lib/css_mixins'

import selectArrow from '../assets/select_arrow.svg'

export const root = styled.div`
  margin-top: 10px;
  color: ${leo.color.light.text.primary};

  --toggle-button-width: 40px;
  --toggle-button-handle-margin: 2px;

  div select {
    width: auto;
    -webkit-appearance: none;
    background:
      url(/${selectArrow}) calc(100% - 11px) center no-repeat,
      ${leo.color.light.container.highlight};
    background-size: 11px;
    border-radius: 8px;
    border: none;
    color: inherit;
    font-size: 12px;
    line-height: 18px;
    padding: 5px 32px 5px 8px;

    &[disabled] {
      background:
        url(/${selectArrow}) calc(100% - 11px) center no-repeat,
        ${leo.color.light.container.disabled};
      background-size: 11px;
      color: ${leo.color.light.text.disabled}
    }
  }
`

export const adTypeGrid = styled.div`
  display: grid;
  grid-template-columns: auto 1fr auto;
  align-items: center;
  gap: 22px;
`

export const adTypeLabel = styled.div`
  font-size: 12px;
  line-height: 18px;
  display: flex;
  align-items: center;
  gap: 9px;
`

export const adTypeInfo = styled.span`
  --leo-icon-size: 14px;
  color: ${leo.color.light.icon.default};

  ${mixins.tooltipAnchor}

  .tooltip {
    top: 50%;
    transform: translateY(-50%);
    left: 100%;
    padding-left: 12px;
  }
`

export const infoTooltip = styled.div`
  ${mixins.tooltipContainer}

  width: 274px;
  padding: 16px;
  font-size: 14px;
  line-height: 24px;
  color: ${leo.color.light.text.primary};
  border-radius: 8px;
  box-shadow: 0px 4px 16px -2px rgba(0, 0, 0, 0.08);
  background: ${leo.color.white};

  &:before {
    height: 9px;
    width: 9px;
    top: calc(50% - 4px);
    left: -4px;
  }
`

export const adTypeControls = styled.div`
  display: flex;
  align-items: center;
  gap: 16px;
`

export const adTypeCount = styled.div`
  justify-self: end;
  font-size: 14px;
  line-height: 24px;
`
