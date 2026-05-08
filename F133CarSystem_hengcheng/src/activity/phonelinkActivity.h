/***********************************************
/gen auto by zuitools
***********************************************/
#ifndef __PHONELINKACTIVITY_H__
#define __PHONELINKACTIVITY_H__


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
#include "window/ZKSlideWindow.h"

/*TAG:Macro宏ID*/
#define ID_PHONELINK_mcStepTextView    50010
#define ID_PHONELINK_apStepTextView    50009
#define ID_PHONELINK_aaStepTextView    50008
#define ID_PHONELINK_tipCancelButton    20006
#define ID_PHONELINK_tipConfirmButton    20005
#define ID_PHONELINK_tipInfoText    50007
#define ID_PHONELINK_tipBgWindow    110007
#define ID_PHONELINK_tipMaskWindow    110006
#define ID_PHONELINK_mcHelpImage    50006
#define ID_PHONELINK_mcWindow    110005
#define ID_PHONELINK_apHelpImage    50005
#define ID_PHONELINK_apWindow    110004
#define ID_PHONELINK_aaHelpImage    50004
#define ID_PHONELINK_aaWindow    110003
#define ID_PHONELINK_cpTipTextView    50003
#define ID_PHONELINK_cpStepTextView    50002
#define ID_PHONELINK_cpHelpImage    50001
#define ID_PHONELINK_cpWindow    110002
#define ID_PHONELINK_miracastTabButton    20004
#define ID_PHONELINK_airplayTabButton    20003
#define ID_PHONELINK_androidTabButton    20002
#define ID_PHONELINK_carplayTabButton    20001
#define ID_PHONELINK_bottomNavWindow    110001
/*TAG:Macro宏ID END*/

class phonelinkActivity : public Activity, 
                     public ZKSeekBar::ISeekBarChangeListener, 
                     public ZKListView::IItemClickListener,
                     public ZKListView::AbsListAdapter,
                     public ZKSlideWindow::ISlideItemClickListener,
                     public EasyUIContext::ITouchListener,
                     public ZKEditText::ITextChangeListener,
                     public ZKVideoView::IVideoPlayerMessageListener
{
public:
    phonelinkActivity();
    virtual ~phonelinkActivity();

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

    virtual bool onTouchEvent(const MotionEvent &ev);

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