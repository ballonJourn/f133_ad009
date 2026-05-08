/***********************************************
/gen auto by zuitools
***********************************************/
#ifndef __MAINACTIVITY_H__
#define __MAINACTIVITY_H__


#include "app/Activity.h"
#include "entry/EasyUIContext.h"

#include "uart/ProtocolData.h"
#include "uart/ProtocolParser.h"

#include "utils/Log.h"
#include "control/ZKDigitalClock.h"
#include "control/ZKButton.h"
#include "control/ZKCircleBar.h"
#include "control/ZKDiagram.h"
#include "control/ZKListView.h"
#include "control/ZKPointer.h"
#include "control/ZKQRCode.h"
#include "control/ZKTextView.h"
#include "control/ZKSeekBar.h"
#include "control/ZKEditText.h"
#include "control/ZKVideoView.h"
#include "control/ZKRadioGroup.h"
#include "window/ZKSlideWindow.h"

/*TAG:Macro宏ID*/
#define ID_MAIN_miracastPage2Button    20009
#define ID_MAIN_toLocalmusicButton    20016
#define ID_MAIN_SeekBar1    91004
#define ID_MAIN_iconBrightnessTextView    50024
#define ID_MAIN_iconVoiceTextView    50023
#define ID_MAIN_voiceSeekBar    91003
#define ID_MAIN_brightnessTextView    50021
#define ID_MAIN_voiceTextView    50022
#define ID_MAIN_leftSetButton    20015
#define ID_MAIN_leftAudioButton    20014
#define ID_MAIN_leftVideoButton    20013
#define ID_MAIN_leftMusicButton    20012
#define ID_MAIN_naviBg    50020
#define ID_MAIN_TextView13    50019
#define ID_MAIN_TextView12    50017
#define ID_MAIN_airplayPage2Button    20011
#define ID_MAIN_aicastPage2Button    20010
#define ID_MAIN_TextView11    50016
#define ID_MAIN_TextView10    50014
#define ID_MAIN_TextView9    50012
#define ID_MAIN_TextView8    50011
#define ID_MAIN_albumPage2Button    20008
#define ID_MAIN_videoPage2Button    20007
#define ID_MAIN_musicPage2Button    20001
#define ID_MAIN_TextView7    50010
#define ID_MAIN_TextView5    50009
#define ID_MAIN_TextView4    50005
#define ID_MAIN_TextView1    50002
#define ID_MAIN_androidautoPage2Button    20006
#define ID_MAIN_audiooutputButton    20005
#define ID_MAIN_carplayPage2Button    20004
#define ID_MAIN_bluetoothPage2Button    20003
#define ID_MAIN_enableButton    20002
#define ID_MAIN_RadioButton1    22002
#define ID_MAIN_RadioButton0    22001
#define ID_MAIN_StatusRadioGroup    94001
#define ID_MAIN_Window7    110003
#define ID_MAIN_Window6    110002
#define ID_MAIN_musictext    50018
#define ID_MAIN_musicTextViewWindow    50008
#define ID_MAIN_musicPicTextView    50004
#define ID_MAIN_Window2    110001
#define ID_MAIN_TextView6    50013
#define ID_MAIN_Window5    110011
#define ID_MAIN_linkTipsWindow    110010
#define ID_MAIN_TextView3    50006
#define ID_MAIN_Window1    110005
#define ID_MAIN_btTipsWindow    110004
#define ID_MAIN_artistTextView    50015
#define ID_MAIN_ToMusic    20021
#define ID_MAIN_PlayProgressSeekbar    91002
#define ID_MAIN_TextView2    50003
#define ID_MAIN_PrevButton    20023
#define ID_MAIN_ButtonPlay    20026
#define ID_MAIN_NextButton    20027
#define ID_MAIN_titleTextView    50001
#define ID_MAIN_musicWindow    110012
#define ID_MAIN_TextViewBg    50007
/*TAG:Macro宏ID END*/

class mainActivity : public Activity, 
                     public ZKSeekBar::ISeekBarChangeListener, 
                     public ZKListView::IItemClickListener,
                     public ZKListView::AbsListAdapter,
                     public ZKSlideWindow::ISlideItemClickListener,
					 public ZKSlideWindow::ISlidePageChangeListener,
                     public EasyUIContext::ITouchListener,
                     public ZKRadioGroup::ICheckedChangeListener,
                     public ZKEditText::ITextChangeListener,
                     public ZKVideoView::IVideoPlayerMessageListener
{
public:
    mainActivity();
    virtual ~mainActivity();

    /**
     * 注册定时器
     */
	void registerUserTimer(int id, int time);
	/**
	 * 取消定时器
	 */
	void unregisterUserTimer(int id);
	/**
	 * 重置定时器
	 */
	void resetUserTimer(int id, int time);

protected:
    /*TAG:PROTECTED_FUNCTION*/
    virtual const char* getAppName() const;
    virtual void onCreate();
    virtual void onClick(ZKBase *pBase);
    virtual void onResume();
    virtual void onPause();
    virtual void onIntent(const Intent *intentPtr);
    virtual bool onTimer(int id);

    virtual void onProgressChanged(ZKSeekBar *pSeekBar, int progress);

    virtual int getListItemCount(const ZKListView *pListView) const;
    virtual void obtainListItemData(ZKListView *pListView, ZKListView::ZKListItem *pListItem, int index);
    virtual void onItemClick(ZKListView *pListView, int index, int subItemIndex);

    virtual void onSlideItemClick(ZKSlideWindow *pSlideWindow, int index);
    virtual void onSlidePageChange(ZKSlideWindow *pSlideWindow, int page);

    virtual bool onTouchEvent(const MotionEvent &ev);
    virtual void onCheckedChanged(ZKRadioGroup* pRadioGroup, int checkedID);

    virtual void onTextChanged(ZKTextView *pTextView, const string &text);

    void rigesterActivityTimer();

    virtual void onVideoPlayerMessage(ZKVideoView *pVideoView, int msg);
    void videoLoopPlayback(ZKVideoView *pVideoView, int msg, size_t callbackTabIndex);
    void startVideoLoopPlayback();
    void stopVideoLoopPlayback();
    bool parseVideoFileList(const char *pFileListPath, std::vector<string>& mediaFileList);
    int removeCharFromString(string& nString, char c);


private:
    /*TAG:PRIVATE_VARIABLE*/
    int mVideoLoopIndex;
    int mVideoLoopErrorCount;

};

#endif