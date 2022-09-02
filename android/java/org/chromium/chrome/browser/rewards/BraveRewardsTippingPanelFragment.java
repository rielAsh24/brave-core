/**
 * Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.chromium.chrome.browser.rewards;;

import android.os.Bundle;

import androidx.fragment.app.Fragment;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.text.Spanned;
import android.text.SpannableString;
import android.text.style.ClickableSpan;
import android.text.TextPaint;
import androidx.annotation.NonNull;
import org.chromium.chrome.browser.app.BraveActivity;
import org.chromium.chrome.browser.customtabs.CustomTabActivity;
import android.text.method.LinkMovementMethod;
import android.text.style.ForegroundColorSpan;
import android.widget.RadioButton;
import android.widget.TextView;
import android.content.res.Resources;
import android.widget.ToggleButton;
import java.math.RoundingMode;
import java.text.DecimalFormat;
import java.util.Locale;
import android.widget.LinearLayout;
import android.util.Log;
import android.view.animation.TranslateAnimation;
import android.widget.ImageView;
import android.content.Context;
import androidx.fragment.app.FragmentManager;
import org.chromium.chrome.browser.BraveRewardsNativeWorker;
import org.chromium.chrome.browser.BraveRewardsSiteBannerActivity;
import org.chromium.chrome.browser.BraveRewardsBalance;
import org.chromium.chrome.browser.BraveRewardsHelper;


import org.chromium.chrome.R;


public class BraveRewardsTippingPanelFragment extends Fragment {

    private int currentTabId_;
    private boolean monthlyContribution;


    private ToggleButton radio_tip_amount[] = new ToggleButton [3];
    private BraveRewardsNativeWorker mBraveRewardsNativeWorker;
    private LinearLayout sendDonationLayout;
    private boolean mTippingInProgress; //flag preventing multiple tipping processes
    private RadioButton monthlyRadioButton;
    private boolean notEnoughFundsShown;
    private double mBatValue;
    private double mUsdValue;
    private double[] mAmounts;

    private final int TIP_SENT_REQUEST_CODE = 2;
    private final int FADE_OUT_DURATION = 500;


    public static BraveRewardsTippingPanelFragment newInstance(int tabId, 
        boolean isMonthlyContribution, double [] amounts) {
        BraveRewardsTippingPanelFragment fragment = new BraveRewardsTippingPanelFragment();

        Bundle args = new Bundle();
        args.putInt(BraveRewardsSiteBannerActivity.TAB_ID_EXTRA, tabId);
        args.putBoolean(BraveRewardsSiteBannerActivity.IS_MONTHLY_CONTRIBUTION, isMonthlyContribution);
        args.putDoubleArray(BraveRewardsSiteBannerActivity.AMOUNTS_ARGS, amounts);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        mBatValue = 0.0;
        mUsdValue = 0.0;
        if (getArguments() != null) {
            currentTabId_ = getArguments().getInt(BraveRewardsSiteBannerActivity.TAB_ID_EXTRA);
            monthlyContribution = getArguments().getBoolean(BraveRewardsSiteBannerActivity.IS_MONTHLY_CONTRIBUTION);
            mAmounts = getArguments().getDoubleArray(BraveRewardsSiteBannerActivity.AMOUNTS_ARGS);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.brave_rewards_tipping_panel, container, false);
        
        init(view);
        updateTermsOfServicePlaceHolder(view);
        initRadioButtons(view);
        setBalanceText(view);
        sendTipButtonClick();
        clickOtherAmounts(view);
        return view;
    }

    private void init(View view) {
        mBraveRewardsNativeWorker = BraveRewardsNativeWorker.getInstance();
        sendDonationLayout = view.findViewById(R.id.send_donation_button);
        monthlyRadioButton = view.findViewById(R.id.monthly_radio_button);
        monthlyRadioButton.setChecked(monthlyContribution);
        notEnoughFundsShown = false;
    }

    private void clickOtherAmounts(View view) {
        TextView otherAmountsText = view.findViewById(R.id.other_amounts);
        otherAmountsText.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                FragmentManager fragmentManager = getParentFragmentManager();
                fragmentManager.beginTransaction()
                        .replace(R.id.conversionFragmentContainer, new BraveRewardsCustomTipFragment(), "custom_tip_fragment")
                        .addToBackStack(null)
                        .commit();
            }
        });
    }


    private void initRadioButtons(View view) {

        int recurrentAmount = (int) mBraveRewardsNativeWorker.GetPublisherRecurrentDonationAmount(
                mBraveRewardsNativeWorker.GetPublisherId(currentTabId_));

        //bind tip amount custom radio buttons
        radio_tip_amount[0] = view.findViewById(R.id.bat_option1);
        radio_tip_amount[0].setTextOff(BraveRewardsHelper.ONE_BAT_TEXT);
        radio_tip_amount[0].setTextOn(BraveRewardsHelper.ONE_BAT_TEXT);
        radio_tip_amount[0].setChecked(false);

        radio_tip_amount[1] = view.findViewById(R.id.bat_option2);
        radio_tip_amount[1].setTextOff(BraveRewardsHelper.FIVE_BAT_TEXT);
        radio_tip_amount[1].setTextOn(BraveRewardsHelper.FIVE_BAT_TEXT);
        radio_tip_amount[1].setChecked(false);

        radio_tip_amount[2] = view.findViewById(R.id.bat_option3);
        radio_tip_amount[2].setTextOff(BraveRewardsHelper.TEN_BAT_TEXT);
        radio_tip_amount[2].setTextOn(BraveRewardsHelper.TEN_BAT_TEXT);
        radio_tip_amount[2].setChecked(false);

        switch (recurrentAmount) {
            case 1:
                radio_tip_amount[0].setChecked(true);
                break;
            case 5:
                radio_tip_amount[1].setChecked(true);
                break;
            case 10:
                radio_tip_amount[2].setChecked(true);
                break;
            default:
                radio_tip_amount[0].setChecked(true);
                break;
        }
        for (ToggleButton tb : radio_tip_amount){
            tb.setOnClickListener(radio_clicker);
        }

        double usdValue = mBraveRewardsNativeWorker.GetWalletRate();
        double fiveBat = 5 * usdValue;
        double tenBat = 10 * usdValue;
        String oneBatRate = String.format(Locale.getDefault(), "%.2f", usdValue) + " USD";
        String fiveBatRate = String.format(Locale.getDefault(), "%.2f", fiveBat) + " USD";
        String tenBatRate = String.format(Locale.getDefault(), "%.2f", tenBat) + " USD";
        ((TextView) view.findViewById(R.id.amount1)).setText(oneBatRate);
        ((TextView) view.findViewById(R.id.amount2)).setText(fiveBatRate);
        ((TextView) view.findViewById(R.id.amount3)).setText(tenBatRate);
    }

    private View.OnClickListener radio_clicker = new View.OnClickListener() {
        @Override
        public void onClick(View view) {
            ToggleButton tb_pressed = (ToggleButton)view;
            if ( tb_pressed.isChecked() == false){
                tb_pressed.setChecked(true);
                return;
            }

            int id = view.getId();
            for (ToggleButton tb : radio_tip_amount){
                if (tb.getId() == id){
                    continue;
                }
                tb.setChecked(false);
            }
        }
    };

    private double selectedAmount() {
        double amount = 0;
        for (ToggleButton tb : radio_tip_amount){
            if (tb.isChecked()) {
                int id = tb.getId();
                if (id == R.id.bat_option1) {
                    amount = 1;
                } else if (id == R.id.bat_option2) {
                    amount = 5;
                } else if (id == R.id.bat_option3) {
                    amount = 10;
                }

                break;
            }
        }
        //Selected amount from custom tip
        BraveRewardsCustomTipConfirmationFragment customTipConfirmationFragment = (BraveRewardsCustomTipConfirmationFragment)getParentFragmentManager().findFragmentByTag("custom_tip_confirmation_fragment");
        Log.i("SujitSujit", "myFragment "+customTipConfirmationFragment);
        if (customTipConfirmationFragment != null && customTipConfirmationFragment.isVisible()) {
            amount = mBatValue;
        }

        return amount;
    }

    private boolean isCustomTipFragmentVisibile() {
        BraveRewardsCustomTipFragment customTipFragment = (BraveRewardsCustomTipFragment)getParentFragmentManager().findFragmentByTag("custom_tip_fragment");
        return customTipFragment != null && customTipFragment.isVisible();
    }

    private void showCustomTipConfirmationFrament() {
        Log.i("SujitSujit", "myFragment1 ");

        if (getParentFragmentManager().getBackStackEntryCount() != 0) {
            getParentFragmentManager().popBackStack();
        }
        resetSendLayoutText();

        FragmentManager fragmentManager = getParentFragmentManager();
        fragmentManager.beginTransaction()
                .replace(R.id.conversionFragmentContainer, 
                    BraveRewardsCustomTipConfirmationFragment.newInstance(mBatValue, mUsdValue), 
                    "custom_tip_confirmation_fragment")
                .addToBackStack(null)
                .commit();
    }

    private void donateAndShowConfirmationScreen(double amount) {
        Log.i("SujitSujit", "myFragment2 ");

        if (mTippingInProgress){
            return;
        }
        mTippingInProgress = true;
        boolean monthly_bool = monthlyRadioButton.isChecked();
        Log.i("SujitSujit", "amount "+amount+" monthly "+monthly_bool);

        mBraveRewardsNativeWorker.Donate(mBraveRewardsNativeWorker.GetPublisherId(currentTabId_),
        amount, monthly_bool);

        replaceTipConfirmationFragment(amount, monthly_bool);
    }

    private void showNotEnoughFunds() {
        ImageView imageView = sendDonationLayout.findViewById(R.id.send_tip_image);
        TextView sendDonationText = sendDonationLayout.findViewById(R.id.send_donation_text);

        //Not funds case toggle the text and image
        if(!notEnoughFundsShown) {
            notEnoughFundsShown = true;
            imageView.setImageResource(R.drawable.icn_frowning_face);
            sendDonationText.setText(String.format(getResources().getString(R.string.brave_ui_not_enough_tokens), getResources().getString(R.string.token)));
        } else {
            notEnoughFundsShown = false;
            imageView.setImageResource(R.drawable.ic_send_tip);
            sendDonationText.setText(getResources().getString(R.string.send_tip));
        }
        int fromY = sendDonationLayout.getHeight();

        TranslateAnimation animate = new TranslateAnimation(0, 0, fromY, 0);
        animate.setDuration(FADE_OUT_DURATION);

        sendDonationLayout.startAnimation(animate);
    }

    private double getBalance() {
        double balance = .0;
        BraveRewardsBalance rewards_balance = mBraveRewardsNativeWorker.GetWalletBalance();
        if (rewards_balance != null){
            balance = rewards_balance.getTotal();
        }
        return balance;
    }

    private void sendTipButtonClick() {
        View.OnClickListener send_tip_clicker = new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                double balance = getBalance();
                double amount = selectedAmount();
                boolean enough_funds = ((balance - amount) >= 0);
               
                //When custom tip with enough funds is there then continue button will show 
                //click on continue button will show CustomTipConfirmation UI
                if (isCustomTipFragmentVisibile() ) {
                    enough_funds = ((balance - mBatValue) >= 0) && mBatValue >= 0.25;
                    if (true == enough_funds) {
                        showCustomTipConfirmationFrament();
                    } // else nothing
                } 
                //proceed to tipping
                else if (true == enough_funds) {
                  donateAndShowConfirmationScreen(amount);
                }
                //if not enough funds
                else {
                    showNotEnoughFunds();
                }
            }
        };

        sendDonationLayout.setOnClickListener(send_tip_clicker);

    }

    public void replaceTipConfirmationFragment(double amount, boolean isMonthly) {
             BraveRewardsTipConfirmationListener confirmation = (BraveRewardsTipConfirmationListener)getActivity();
             confirmation.onTipConfirmation(amount, isMonthly);
    }

    private void updateTermsOfServicePlaceHolder(View view) {
        Resources res = getResources();
        TextView proceedTextView = view.findViewById(R.id.proceed_terms_of_service);
        proceedTextView.setMovementMethod(LinkMovementMethod.getInstance());
        String termsOfServiceText = String.format(res.getString(R.string.brave_rewards_tos_text),
                res.getString(R.string.terms_of_service),
                res.getString(R.string.privacy_policy));


        SpannableString spannableString = stringToSpannableString(termsOfServiceText);
        proceedTextView.setText(spannableString);
    }

    private SpannableString stringToSpannableString( String text) {
        Spanned textSpanned = BraveRewardsHelper.spannedFromHtmlString(text);
        SpannableString textSpannableString = new SpannableString(textSpanned.toString());
        setSpan(text, textSpannableString, R.string.terms_of_service, termsOfServiceClickableSpan); // terms of service
        setSpan(text, textSpannableString, R.string.privacy_policy, privacyPolicyClickableSpan); // privacy policy
        return textSpannableString;
    }

    private void setSpan(String text, SpannableString tosTextSS, int stringId, ClickableSpan clickableSpan) {
        String spanString = getResources().getString(stringId);
        int spanLength = spanString.length();
        int index = text.indexOf(spanString);

        tosTextSS.setSpan(clickableSpan, index, index + spanLength, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        tosTextSS.setSpan(new ForegroundColorSpan(getResources().getColor(R.color.brave_rewards_modal_theme_color)),
                index, index + spanLength, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
    }

    private ClickableSpan termsOfServiceClickableSpan = new ClickableSpan() {
        @Override
        public void onClick(@NonNull View textView) {
            CustomTabActivity.showInfoPage(getActivity(), BraveActivity.BRAVE_TERMS_PAGE);
        }
        @Override
        public void updateDrawState(@NonNull TextPaint ds) {
            super.updateDrawState(ds);
            ds.setUnderlineText(false);
        }
    };

    private ClickableSpan privacyPolicyClickableSpan = new ClickableSpan() {
        @Override
        public void onClick(@NonNull View textView) {
            CustomTabActivity.showInfoPage(getActivity(), BraveActivity.BRAVE_PRIVACY_POLICY);
        }
        @Override
        public void updateDrawState(@NonNull TextPaint ds) {
            super.updateDrawState(ds);
            ds.setUnderlineText(false);
        }
    };

    void setBalanceText(View view) {
        double balance = .0;
        BraveRewardsBalance rewards_balance = mBraveRewardsNativeWorker.GetWalletBalance();
        if (rewards_balance != null){
            balance = rewards_balance.getTotal();
        }

        DecimalFormat df = new DecimalFormat("#.###");
        df.setRoundingMode(RoundingMode.FLOOR);
        df.setMinimumFractionDigits(3);
        String walletAmount = df.format(balance) + " " + BraveRewardsHelper.BAT_TEXT;

        ((TextView) view.findViewById(R.id.wallet_amount_text)).setText(walletAmount);
    }

    public void updateAmount(double batValue, double usdValue) {
        mBatValue = batValue;
        mUsdValue = usdValue;
        double balance = .0;
        Log.i("SujitSujit", "BraveRewardsTippingPanelFragment amount "+batValue);
        BraveRewardsBalance rewards_balance = mBraveRewardsNativeWorker.GetWalletBalance();
        if (rewards_balance != null) {
            balance = rewards_balance.getTotal();
        }
        if(batValue >= 0.25  && batValue <= balance) {
            ((TextView) sendDonationLayout.findViewById(R.id.send_donation_text)).setText(getResources().getString(R.string.continue_tip));
        } else if(batValue >= balance) {
            ((TextView) sendDonationLayout.findViewById(R.id.send_donation_text)).setText(getResources().getString(R.string.not_enough_fund));
        } else if(batValue < 0.25)  {
            ((TextView) sendDonationLayout.findViewById(R.id.send_donation_text)).setText(getResources().getString(R.string.minimum_tip_amount));
        } else {
            ((TextView) sendDonationLayout.findViewById(R.id.send_donation_text)).setText(getResources().getString(R.string.send_tip));
        }
    }

    public void resetSendLayoutText() {
        Log.i("SujitSujit", "resetSendLayoutText ");
        ((TextView) sendDonationLayout.findViewById(R.id.send_donation_text)).setText(getResources().getString(R.string.send_tip));
    }
}