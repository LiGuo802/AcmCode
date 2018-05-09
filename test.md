## 一、接听界面为：AnswerFragment，其内部只有一个控件IncomingWidget（继承自RelativeLayout），包括以下元素：

![](https://i.imgur.com/L8grHv9.jpg)

## 二、AnswerPresenter接收通话状态的改变，并通知AnswerFragment进行界面刷新

```java
public class AnswerPresenter extends... implements ... {

    // 有新来电
    private void processIncomingCall(Call call) {
        ...
        getUi().showAnswerUi(true, true);
    }

    
    // 通话状态改变
    public void onCallChanged(Call call) {
        ...
        if (call.getState() != Call.State.INCOMING) {
            final Call incall = CallList.getInstance().getIncomingCall();
            if (incall != null) {
                getUi().showAnswerUi(true, true);
            } else {
                getUi().showAnswerUi(false, true);
            }
        }
        ...
    }

    // 有电话挂断了
    public void onDisconnect(Call call) {
        ...
        // 建立通话，再来电，此时显示3个按钮的界面。如果正在通话的那路挂断了，恢复显示正常的来电界面
        Call incomingCall = CallList.getInstance().getIncomingCall();
        if(!mAnswerAndDeclineClick){
            if (call != incomingCall && incomingCall != null) {
                getUi().showAnswerUi(true, true);
            }
        }else{
            if(incomingCall != null){
                TelecomAdapter.getInstance().answerCall(incomingCall.getId(), VideoProfile.STATE_AUDIO_ONLY);
            }
            mAnswerAndDeclineClick = false;
        }
        ...
    }
}
```

可以看到所有都调用到了showAnswerUi方法。

## 三、AnswerFragment的show方法
```java
    /**
     * @param show 是否显示AnswerFragment
     * @param animate 是否要做动画
     */
    public void showAnswerUi(boolean show, boolean animate) {
        View view = getView();
        if (view == null){
            return;
        }

        // 如果不显示的话，直接设为Gone
        view.setVisibility(show ? View.VISIBLE : View.GONE);
        if (mIncomingCallWidget == null) {
            return;
        }
        Log.d(TAG, "Show answer UI: " + show);
        if (show) {
            // 省略摩托车模式处理
            boolean showSecIncomingUI = showSecIncomingUI(); // 通话通话状态判断 是否显示二路来电界面
            boolean showVideoIncomingUI = !showSecIncomingUI && showVideoIncomingUI(); // 判断是否为视频来电
            if (!mIncomingShowing || mshowSecIncomingUI != showSecIncomingUI || mShowVideoIncomingUI != showVideoIncomingUI) {
                mIncomingShowing = true;
                mIncomingCallWidget.setTouchEnabled(false);
                mshowSecIncomingUI = showSecIncomingUI;
                mShowVideoIncomingUI = showVideoIncomingUI;

                // 重置界面
                mIncomingCallWidget.reset(mshowSecIncomingUI, mShowVideoIncomingUI);

                // 开启动画
                mIncomingCallWidget.startFirstAnimation();
            }
        } else {
            // 省略部分逻辑
            mshowSecIncomingUI = false;
            mShowVideoIncomingUI = false;
            mIncomingShowing = false;
            mIncomingCallWidget.reset(mshowSecIncomingUI, mShowVideoIncomingUI);
            if(mHandler.hasMessages(RESET_INCOMING_WIGHT)){
                mHandler.removeMessages(RESET_INCOMING_WIGHT);
            }
        }
    }
```

## 四、重置界面

```java
    public void reset(boolean showSecIncomingUI, boolean showVideoIncomingUI) {

        // 是否为超级省电模式或者移动入围版
        mPowerSave = SystemProperties.getBoolean("sys.super_power_save", false) || CallUtils.IS_CMCC;

        // 判断当前是否有动画正在运行，有的话先取消掉
        if (mFirstAniRunning && mFirstAniSet != null) {
            mFirstAniSet.cancel();
        }
        mFirstAniRunning = false;

        // 停止律动效果
        stopWaveDirectly();

        mShowSecIncomingUI = showSecIncomingUI;
        mShowVideoIncomingUI = showVideoIncomingUI;

        if(mShowVideoIncomingUI) {
            // 视频来电处理
            mAnswer.setImageResource(R.drawable.ic_video_answer);
            mHangUp.setImageResource(R.drawable.ic_video_reject);
            mVideoSwitchButton.setVisibility(View.VISIBLE);
            mVideoSwitchText.setVisibility(View.VISIBLE);
            mVideoSwitchButton.setAlpha(1f);
            mVideoSwitchText.setAlpha(1f);
            mVideoSwitchButton.setEnabled(true);
            mVideoSwitchButton.showSwitchVoice();
            mShowVideoAnswer = true;
            mVideoSwitchText.setText(R.string.video_voice_answer);
        } else {
            // 语音来电处理
            mAnswer.setImageResource(R.drawable.ic_voice_answer);
            mHangUp.setImageResource(R.drawable.ic_voice_reject);
            mVideoSwitchButton.setVisibility(View.GONE);
            mVideoSwitchText.setVisibility(View.GONE);
            mShowVideoAnswer = false;
        }

        // 初始化所有控件状态
        mSoundWave.setVisibility(View.VISIBLE);
        for (ImageView iv : mSoundWaveCircles) {
            iv.setColorFilter(Color.WHITE);
            iv.setAlpha(0f);
        }
        mSoundWave.setAlpha(1f);
        mArrowUp1.setAlpha(0f);
        mArrowUp2.setAlpha(0f);
        mArrowUp3.setAlpha(0f);
        mArrowDown1.setAlpha(0f);
        mArrowDown2.setAlpha(0f);
        mArrowDown3.setAlpha(0f);
        mAnswer.setAlpha((!mShowSecIncomingUI && !mShowVehicleModeUI) ? 1f : 0f);
        mHangUp.setAlpha((!mShowSecIncomingUI && !mShowVehicleModeUI) ? 1f : 0f);


        if(!mShowSecIncomingUI){
            mPhotoContainer.setVisibility(View.VISIBLE);
            mPhotoContainer.setAlpha(1f);
            mPhotoContainer.setScaleX(1f);
            mPhotoContainer.setScaleY(1f);
            mPhotoContainer.setTranslationY(0);

            // 隐藏二路来电控件
            keepAndAnswer.setVisibility(View.GONE);
            rejectAndAnswer.setVisibility(View.GONE);
            reject.setVisibility(View.GONE);

            // 显示短信拒接、来电提醒界面
            mRejectCallWithMsg.setVisibility(View.VISIBLE);
            mRejectCallWithMsg.setAlpha(1.0f);
            mRemindCallLaterBnt.setVisibility(View.VISIBLE);
            mRemindCallLaterBnt.setAlpha(1.0f);
        }else{
            // 隐藏滑动接听控件等
        }

        // 更新自身alpha为1
        updateAlpha(1f);
        
        // 关闭两个弹窗
        closeRejectMessagesView(CLOSE_MSG_DIALOG_NORMAL);
        closeRemindCallView();
        
        mAnimStepMark = ANIM_STEP_INIT;
        mMessagesOpened = false;
        mRemindOpened = false;
        mTouchEnabled = true;
        mAlreadyRespond = false;
        mTouchAtLeastTwoPoint = false;
        mDownHit = false;
    }
```

## 五、开启动画

```java
    public void startFirstAnimation() {
        if (mFirstAniRunning) {
            // 如果动画已经运行了，返回
            return;
        }
        reset(mShowSecIncomingUI, mShowVideoIncomingUI); // 重复操作，正在评估能否删除

        // 超级省电模式等，直接将“提醒”“拒接”按钮显示出来
        if (mPowerSave || ...) {
            mRemindCallLaterBnt.setAlpha(1f);
            mRejectCallWithMsg.setAlpha(1f);
        }

        if(...){
            // 其他模式处理
            return;
        }
        
        if(!mShowSecIncomingUI){
            // 普通来电动效
            startIncomingAnimation();
        }else{
            // 二路来电动效
            startSecIncomingAnimation();
        }
    }
```

## 六、普通来电动效

```java
    private void startIncomingAnimation() {
        if (mPowerSave) {
            // 超级省电模式处理
            mWaveRunning = true;
            mCircleWaveRunning = true;
            onPhotoChanged();
            return;
        }

        mWaveRunning = true;
        mCircleWaveRunning = true;
        updateWaveAnim();// 更新小点的律动动效
        updateCircleWaveAnim(); // 更新波纹的律动动效

        // 开始动效
        AnimatorSet animatorSet = new AnimatorSet();
        animatorSet.playTogether(mWaveAnimatorSet,mCircleWaveAnimatorSet);
        animatorSet.start();
    }
```

```java
    /**
     * 更新六个小点律动动画
     */
    private void updateWaveAnim() {
        List<Animator> animators = new ArrayList<Animator>();

        if (mPowerSave) {
            // 超级省电直接设置为最终效果
        } else {
            // 添加六个小点的律动动画
            animators.add(AnimateUtils.getWaveAnim(0, mArrowUp3, mArrowDown1)); // 子动画1
            animators.add(AnimateUtils.getWaveAnim(250, mArrowUp2, mArrowDown2));
            animators.add(AnimateUtils.getWaveAnim(500, mArrowUp1, mArrowDown3));

            if(mShowVideoIncomingUI) {
                animators.add(AnimateUtils.getAppearAnim(200, 500, mVideoSwitchButton.getAlpha(), 1f, 0, 0, mVideoSwitchButton, mVideoSwitchText));
            }
        }

        // 取消如果存在的滑动按钮消失动效
        if(mSoundWaveDismissAnim != null){
            mSoundWaveDismissAnim.cancel();
        }

        // 滑动按钮出现动画
        mSoundWaveAppearAnim = AnimateUtils.getAlphaAnimator((long) (100 * (1 - mSoundWave.getAlpha())), mSoundWave.getAlpha(), 1, mSoundWave);
        animators.add(mSoundWaveAppearAnim);
        CallUtils.debugAnimateThread();

        mWaveAnimatorSet = new AnimatorSet();
        mWaveAnimatorSet.playTogether(animators);
    }
```
```
    /**
     * 更新三个细圈律动动画
     */
    private void updateCircleWaveAnim() {
        List<Animator> animatorCircleWaves = new ArrayList<Animator>();

        if (mPowerSave) {
            mSoundWaveCircles[0].setAlpha(0f);
            mSoundWaveCircles[1].setAlpha(0.6f);
            mSoundWaveCircles[1].setScaleX(41f / 63);
            mSoundWaveCircles[1].setScaleY(41f / 63);
            mSoundWaveCircles[2].setAlpha(0.3f);
            mSoundWaveCircles[2].setScaleX(52f / 63);
            mSoundWaveCircles[2].setScaleY(52f / 63);
            mSoundWaveCircles[3].setAlpha(0.1f);
            mSoundWaveCircles[3].setScaleX(1f);
            mSoundWaveCircles[3].setScaleY(1f);
        } else {
            for (int i = 0; i < 4; i++) {
                // 子动画3
                animatorCircleWaves.add(AnimateUtils.getWaveCircleAnim(i * 300, mSoundWaveCircles[i]));
            }
        }

        CallUtils.debugAnimateThread();

        mCircleWaveAnimatorSet = new AnimatorSet();
        mCircleWaveAnimatorSet.playTogether(animatorCircleWaves);
    }
```

#### 子动画1：小点的律动动画
```java
    public static ValueAnimator getWaveAnim(long delay, final View... views) {
        ValueAnimator animator = ValueAnimator.ofFloat(0f, 1.75f);
        animator.addUpdateListener(new AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                float f = (Float) animation.getAnimatedValue();
                float alpha = 0;

                if (f <= 0.5f) {            // 500ms内由 0 -> 1
                    alpha = f * 2;
                } else if (f <= 0.75f) {    // 变为1后保持250ms
                    alpha = 1f;
                } else if (f <= 1.5f) {     // 750ms内由 1 -> 0
                    alpha = 2 - 4 * f / 3;
                } else {                    // 消失后250ms再开始下次动画
                    alpha = 0f;
                }

                if (views != null) {
                    for (View view : views) {
                        view.setAlpha(alpha);
                    }
                }
            }
        });
        animator.setDuration(1750);
        animator.setStartDelay(delay);
        animator.setRepeatCount(ValueAnimator.INFINITE);
        animator.setRepeatMode(ValueAnimator.RESTART);
        return animator;
    }
```

#### 子动画2：滑动按钮出现动画
```java
    public static Animator getAlphaAnimator(long duration, float from, float to, View target) {
        ObjectAnimator a = ObjectAnimator.ofFloat(target, "alpha", from, to);
        a.setDuration(duration);
        return a;
    }
```

#### 子动画3：
```java
    /**
     * 来电界面-滑动接听时，波纹扩散动画
     */
    public static ValueAnimator getWaveCircleAnim(long delay, final View view){

        float scaleStart = 0.45f, scaleEnd = 1f;
        float scaleDelay = scaleEnd - scaleStart;
        PathInterpolator alphaIncPI = new PathInterpolator(0.12f, 0.31f, 0.51f, 1.00f);
        PathInterpolator alphaDecPI = new PathInterpolator(0.25f, 0.10f, 0.25f, 1.00f);
        PathInterpolator scalePI = new PathInterpolator(0.00f, 0.00f, 0.67f, 1.00f);

        ValueAnimator animator = ValueAnimator.ofFloat(0f, 1.2f);

        animator.addUpdateListener(new AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator anim) {
                float fraction = anim.getAnimatedFraction();

                if(fraction > 1.0){ // 延迟200ms
                    return;
                }

                float alpha, scale;
                if (fraction <= 0.15) { // alpha: 0 -> 1
                    alpha = alphaIncPI.getInterpolation(fraction / 0.15f);
                } else { // alpha: 1 -> 0
                    alpha = 1f - alphaDecPI.getInterpolation((fraction - 0.15f) / 0.85f);
                }

                scale = scaleStart + scalePI.getInterpolation(fraction) * scaleDelay;

                view.setAlpha(alpha);
                view.setScaleX(scale);
                view.setScaleY(scale);
                view.requestLayout();
            }
        });
        animator.setDuration(1200);
        animator.setStartDelay(delay);
        animator.setRepeatMode(ValueAnimator.RESTART);
        animator.setRepeatCount(ValueAnimator.INFINITE);
        return animator;
    }

```

## 七、拖动与颜色改变：
```java
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (!mTouchEnabled || mInCallActivity == null) {
            Log.w(TAG, "return mTouchEnabled : " + mTouchEnabled+",mInCallActivity is Null : "+(mInCallActivity == null));
            return false;
        }
        int action = event.getAction();

        if (action == MotionEvent.ACTION_DOWN) {
            mIsValidDownEvent = true;
        }
        if (mMessagesOpened) {
            // 短信拒接弹窗打开时的事件处理
            return true;
        }
        if (mRemindOpened) {
            // 提醒弹窗打开时的事件处理
            return true;
        }

        if (mGestureDetector != null) {
            if (mGestureDetector.onTouchEvent(event)) {
                Log.i(TAG, "onTouchEvent  ：mGestureDetector.onTouchEvent");
                return true; // only "onFling" may return true, that indicates fling success!
            }
        }
        switch (action) {
            case MotionEvent.ACTION_DOWN:
                handleDown(event);
                break;
            case MotionEvent.ACTION_MOVE:
                handleMove(event);
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                handleUp(event);
                break;
            // 防误触
            // 比如，左手往挂断方向轻轻滑一点距离，松开左手同时，右手点击屏幕上方，就自动挂断了
            case MotionEvent.ACTION_POINTER_UP:
                Log.i(TAG,
                        "set mTouchAtLeastTwoPoint = true. action = MotionEvent.ACTION_POINTER_UP");
                mTouchAtLeastTwoPoint = true;
                break;
        }
        return true;
    }
```

```java
    // 重置一些状态值
    private void resetState() {
        mTouchEnabled = true; // 是否可触摸
        mMessagesOpened = false; // 短信拒接弹窗是否打开
        mRemindOpened = false;  // 提醒弹窗是否打开
        ...
    }
    
    // down事件处理
    private void handleDown(MotionEvent event) {
        mDownPosY = event.getY();
        getCenterPos();
        Log.i(TAG, "handledown....   mDownPosY = " + mDownPosY + ", mCenterPos : " + mCenterPos);
        resetState();

        if (mDownPosY < (mCenterPos + mDownOffset) && mDownPosY > (mCenterPos - mDownOffset) && !mShowSecIncomingUI && !mShowVehicleModeUI) {
            // 点击事件合法时，停止小点的律动动画，开始中间粗圆的消失动画 （子动画2）           
            mDownHit = true;
            stopWave();
        }
        ...
    }

    private void handleMove(MotionEvent event) {
        if (mDownHit && !mTouchAtLeastTwoPoint) {
            int move = (int) (event.getY() - mDownPosY);
            if (move > mMaxDistance) {
                move = mMaxDistance;
            } else if (move < -mMaxDistance) {
                move = -mMaxDistance;
            }
            // 更新细圈的颜色
            updatePhotoView(move); // 子动画1
        }
        ...
    }

    private void handleUp(MotionEvent event) {
        if (mDownHit && !mTouchAtLeastTwoPoint && event.getY() - mDownPosY > mMinMoveDistance) {
            mVideoSwitchButton.setEnabled(false);
            // 接听
            answerTriggerd();
        } else if (mDownHit && !mTouchAtLeastTwoPoint
                && mDownPosY - event.getY() > mMinMoveDistance) {
            // 拒接
            rejectTriggerd();
        } else {
            // 开启小点的律动动画，开始中间粗圆的出现动画 （子动画3）
            startWave();
        }

        mTouchAtLeastTwoPoint = false;
    }
```

#### 子动画1：颜色改变
```java
    private void updatePhotoView(float distance) {
        // 计算滑动比例
        float ratio = (float) Math.abs(distance) / mMaxDistance;

        // 将对端图标逐渐变为透明，计算细圈颜色
        int waveCircleColor;
        if (distance > 0) { // 下滑接听
            waveCircleColor = (int) mCircleWaveArgbEvaluator.evaluate(ratio, Color.WHITE, ANSWER_COLOR);
            mAnswer.setAlpha(1f);
            mHangUp.setAlpha(1f - ratio);
        } else { // 上滑拒接
            waveCircleColor = (int) mCircleWaveArgbEvaluator.evaluate(ratio, Color.WHITE, REJECT_COLOR);
            mHangUp.setAlpha(1f);
            mAnswer.setAlpha(1f - ratio);
        }

        // 改变细圈颜色
        for (ImageView iv : mSoundWaveCircles) iv.setColorFilter(waveCircleColor);

        // 改变可拖动按钮的位置
        mPhotoContainer.setTranslationY(distance);
        ...
    }
```

#### 子动画2：律动小点消失动画、中间粗圆消失动画
```java
    public void stopWave() {
        if (mWaveRunning) {
            // 取消小点律动动画
            if (mWaveAnimatorSet != null)
                mWaveAnimatorSet.cancel();
            
            // 取消中间粗圆的出现动画
            if (mSoundWaveAppearAnim != null) {
                mSoundWaveAppearAnim.cancel();
            }

            // 执行取消动画
            updateCancelWaveAnim();
            mWaveCancelAnimatorSet.start();
            mWaveRunning = false;
        }
    }

    private void updateCancelWaveAnim() {
        Animator animator1 = AnimateUtils.getDisappearAnim(mArrowUp1, mArrowUp2, mArrowUp3, mArrowDown1, mArrowDown2, mArrowDown3);

        mSoundWaveDismissAnim = AnimateUtils.getAlphaAnimator((long) (100 * mSoundWave.getAlpha()), mSoundWave.getAlpha(), 0f, mSoundWave);

        CallUtils.debugAnimateThread();

        mWaveCancelAnimatorSet = new AnimatorSet();
        mWaveCancelAnimatorSet.playTogether(animator1, mSoundWaveDismissAnim); // 中间粗圆的消失动画
    }
```

#### 子动画3：律动小点出现动画，中间粗圆出现动画
```java
    public void startWave() {

        // 异常状态过滤：
        if(mShowSecIncomingUI || mShowVehicleModeUI){
            return;
        }
        InCallState state = InCallPresenter.getInstance().getInCallState();
        if(state != InCallState.INCOMING){
            Log.w(TAG, "There is no incoming call when startWave, do NOT start animation!   state=" + state);
            return;
        }
        

        ArrayList<Animator> waveAnimators = new ArrayList<>();

        if (!mWaveRunning) {
            if (mWaveCancelAnimatorSet != null && mWaveCancelAnimatorSet.isRunning()) {
                mWaveCancelAnimatorSet.cancel();
            }
            updateWaveAnim();
            mWaveRunning = true;
            waveAnimators.add(mWaveAnimatorSet);
        }

        if (!mCircleWaveRunning) {
            updateCircleWaveAnim();
            mCircleWaveRunning = true;
            waveAnimators.add(mCircleWaveAnimatorSet);
        }

        AnimatorSet waveAnimatorsSet = new AnimatorSet();
        waveAnimatorsSet.playTogether(waveAnimators);
        waveAnimatorsSet.start();
    }
```
