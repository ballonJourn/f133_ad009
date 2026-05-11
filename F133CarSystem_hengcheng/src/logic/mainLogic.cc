#pragma once
#include "uart/ProtocolSender.h"
/*
*此文件由GUI工具生成
*文件功能:用于处理用户的逻辑相应代码
*功能说明:
*========================onButtonClick_XXXX
当页面中的按键按下后系统会调用对应的函数,XXX代表GUI工具里面的[ID值]名称,
如Button1,当返回值为false的时候系统将不再处理这个按键,返回true的时候系统将会继续处理此按键。比如SYS_BACK.
*========================onSlideWindowItemClick_XXXX(int index)
当页面中存在滑动窗口并且用户点击了滑动窗口的图标后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称,
如slideWindow1;index 代表按下图标的偏移值
*========================onSeekBarChange_XXXX(int progress)
当页面中存在滑动条并且用户改变了进度后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称,
如SeekBar1;progress 代表当前的进度值
*========================ogetListItemCount_XXXX()
当页面中存在滑动列表的时候,更新的时候系统会调用此接口获取列表的总数目,XXX代表GUI工具里面的[ID值]名称,
如List1;返回值为当前列表的总条数
*========================oobtainListItemData_XXXX(ZKListView::ZKListItem *pListItem, int index)
 当页面中存在滑动列表的时候,更新的时候系统会调用此接口获取列表当前条目下的内容信息,XXX代表GUI工具里面的[ID值]名称,
如List1;pListItem 是贴图中的单条目对象,index是列表总目的偏移量。具体见函数说明
*========================常用接口===============
*LOGD(...)  打印调试信息的接口
*mText1Ptr->setText("****") 在控件TextXXX上显示文字****
*mButton1Ptr->setSelected(true); 将控件mButton1设置为选中模式,图片会切换成选中图片,按钮文字会切换为选中后的颜色
*mSeekBarPtr->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1Ptr->refreshListView() 让mListView1 重新刷新,当列表数据变化后调用
*mDashbroadView1Ptr->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 "alt + /"  快捷键可以打开智能提示
*/

#include "net/context.h"
#include "link/context.h"
#include "uart/context.h"
#include "bt/context.h"
#include "media/audio_context.h"
#include "media/media_context.h"
#include "media/music_player.h"
#include "media/media_parser.h"
#include "system/setting.h"
#include "system/fm_emit.h"
#include "system/reverse.h"
#include "manager/LanguageManager.h"
#include "manager/ConfigManager.h"
#include "storage/StoragePreferences.h"
#include "misc/storage.h"
#include "fy/files.hpp"
#include "net/NetManager.h"
#include "os/MountMonitor.h"
#include "system/usb_monitor.h"
#include "tire/tire_parse.h"
#include "sysapp_context.h"
#include "utils/BitmapHelper.h"
#include <base/ui_handler.h>
#include "system/hardware.h"
#include <base/mount_notification.h>
#include <base/time.hpp>
#include "utils/TimeHelper.h"
#include "mode_observer.h"
#include "mcu_hash_checker.h"

#define WIFIMANAGER			NETMANAGER->getWifiManager()

// extern函数声明 - 同步其他界面的SeekBar
extern void set_navibar_brightnessBar(int progress);
extern void set_navibar_PlayVolSeekBar(int progress);
extern void set_ctrlbar_lightSeekBar(int progress);
extern void set_ctrlbar_volumSeekBar(int progress);
extern void setSettingFtu_BrillianceSeekBar(int progress);
extern void setSettingFtu_MediaSeekBar(int progress);

// 主界面SeekBar加载状态
static bool main_seekbar_isLoad = false;

// 外部调用函数 - 设置主界面亮度SeekBar
void set_main_brightnessSeekBar(int progress) {
	if (main_seekbar_isLoad && mSeekBar1Ptr) {
		mSeekBar1Ptr->setProgress(progress);
	}
}

// 外部调用函数 - 设置主界面音量SeekBar
void set_main_voiceSeekBar(int progress) {
	if (main_seekbar_isLoad && mvoiceSeekBarPtr) {
		mvoiceSeekBarPtr->setProgress(progress);
	}
}

#define KEY_LONG_PRESS_TIMEOUT    3000
#define TIMER_POWERKEY_EVENT 	303
#define TIMER_POWERKEY_OFF		304

#define QUERY_LINK_AUTH_TIMER	3
#define SWITCH_ADB_TIMER	4
#define BT_MUSIC_ID3		5
#define MCU_AUTO_UPGRADE	6
#define MUSIC_ERROR_TIMER	20

#define TIME_SYNCED_FLAG "/data/.time_synced_flag"
#define NIGHT_MODE_FLAG "/data/.night_mode_flag"  // 黑夜模式标志文件

#define SCREEN_WIDTH	1024  // 根据实际屏幕宽度调整 (用于滑动切换窗口)

extern void fold_statusbar();

static bt_cb_t _s_bt_cb;
static bool _s_need_reopen_linkview;
static int key_sec = 0;

// 添加页面状态管理
static int _current_page_index = 0;  // 当前页面索引
static bool _is_in_sub_pages = false; // 是否在子页面中

// **内存优化:添加状态管理变量**
static bool _is_in_reverse_mode = false;      // 倒车状态
static bool _is_ui_update_paused = false;     // UI更新暂停状态
static bool _is_exiting_reverse = false;      // 标记是否正在从倒车模式退出

// 白天/黑夜模式状态变量
static bool _is_night_mode = false;           // 当前是否为黑夜模式

// 判断是否为黑夜模式
static bool is_night_mode() {
    return FILE_EXIST(NIGHT_MODE_FLAG);
}

// 设置黑夜模式
static void set_night_mode(bool enabled) {
    if (enabled) {
        system("touch " NIGHT_MODE_FLAG);
        _is_night_mode = true;
        LOGD("[main] Night mode enabled");
    } else {
        system("rm -f " NIGHT_MODE_FLAG);
        _is_night_mode = false;
        LOGD("[main] Night mode disabled (Day mode)");
    }
}

// 初始化白天/黑夜模式状态
static void init_day_night_mode() {
    _is_night_mode = is_night_mode();

    if (_is_night_mode) {
        LOGD("[main] Initializing in night mode");
        if (menableButtonPtr) {
            menableButtonPtr->setSelected(true);
        }
    } else {
        LOGD("[main] Initializing in day mode");
        if (menableButtonPtr) {
            menableButtonPtr->setSelected(false);
        }
    }
}

// 更新所有控件的背景图片(根据白天/黑夜模式)
static void update_all_backgrounds_for_mode() {
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    LOGD("[main] update_all_backgrounds_for_mode() ENTER");

    bitmap_t *bg_bmp = NULL;
    const char *mode_suffix = _is_night_mode ? "_night" : "";

    std::string navi_bg_path =  _is_night_mode ?
            "/HomePage/navi_bg_night.png" : "/HomePage/navi_bg.png";
    std::string navi_icon_tf_path = "/HomePage/navi_icon_tf_n.png";        // TF Card 按钮图标
    std::string navi_icon_reverse_path = "/HomePage/navi_icon_video_n.png"; // 倒车设置图标（暂用 video 图标，待替换）
    std::string navi_icon_audio_path = "/HomePage/navi_icon_audio_n.png";
    std::string navi_icon_set_path = "/HomePage/navi_icon_settings_n.png";
	if (mnaviBgPtr) {
		mnaviBgPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(navi_bg_path.c_str()).c_str());
		mleftTFCardButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(navi_icon_tf_path.c_str()).c_str());
		mleftReverseSettingButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(navi_icon_reverse_path.c_str()).c_str());
		mleftAudioButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(navi_icon_audio_path.c_str()).c_str());
		mleftSetButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(navi_icon_set_path.c_str()).c_str());
	}
    std::string voice_progress_path =  _is_night_mode ?
            "/HomePage/navi_bg_night.png" : "/HomePage/navi_bg.png";
    // 1. 主背景
    if (_is_night_mode) {
        BitmapHelper::loadBitmapFromFile(bg_bmp, CONFIGMANAGER->getResFilePath("/HomePage/carmain_home_wallpaper_night.jpg").c_str(), 3);
        LOGD("[main] Setting night mode background");
    } else {
        BitmapHelper::loadBitmapFromFile(bg_bmp, CONFIGMANAGER->getResFilePath("/HomePage/carmain_home_wallpaper.jpg").c_str(), 3);
        LOGD("[main] Setting day mode background");
    }
    if (mTextViewBgPtr) {
        mTextViewBgPtr->setBackgroundBmp(bg_bmp);
    }

    // 2. musicTextViewWindow - media_bg
    std::string media_bg_path = _is_night_mode ?
        "/HomePage/media_bg_n_night.png" : "/HomePage/media_bg_n.png";
    if (mmusicTextViewWindowPtr) {
        mmusicTextViewWindowPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(media_bg_path.c_str()).c_str());
    }

    // 3. androidautoPage2Button - dock_android_auto
    std::string android_auto_path = _is_night_mode ?
        "/HomePage/dock_android_auto_n_night.png" : "/HomePage/dock_android_auto_n.png";
    if (mandroidautoPage2ButtonPtr) {
        mandroidautoPage2ButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(android_auto_path.c_str()).c_str());
    }

    // 4. carplayPage2Button - dock_carplay
    std::string carplay_path = _is_night_mode ?
        "/HomePage/dock_carplay_n_night.png" : "/HomePage/dock_carplay_n.png";
    if (mcarplayPage2ButtonPtr) {
        mcarplayPage2ButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(carplay_path.c_str()).c_str());
    }

    // 5. audiooutputButton - dock_audio_out
    std::string audio_out_path = _is_night_mode ?
        "/HomePage/dock_audio_out_n_night.png" : "/HomePage/dock_audio_out_n.png";
    if (maudiooutputButtonPtr) {
        maudiooutputButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(audio_out_path.c_str()).c_str());
    }

    // 6. musicPage2Button - dock_music
    std::string music_path = _is_night_mode ?
        "/HomePage/dock_music_n_night.png" : "/HomePage/dock_music_n.png";

    // 7. videoPage2Button - dock_video
    std::string video_path = _is_night_mode ?
        "/HomePage/dock_video_n_night.png" : "/HomePage/dock_video_n.png";

    // 8. albumPage2Button - dock_picture
    std::string picture_path = _is_night_mode ?
        "/HomePage/dock_picture_n_night.png" : "/HomePage/dock_picture_n.png";

    // 9. settingPage2Button - dock_settings
//    std::string settings_path = _is_night_mode ?
//        "/HomePage/dock_settings_n_night.png" : "/HomePage/dock_settings_n.png";
//    if (msettingPage2ButtonPtr) {
//        msettingPage2ButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(settings_path.c_str()).c_str());
//    }

    std::string airplay_path = _is_night_mode ?
        "/HomePage/dock_airplay_n_night.png" : "/HomePage/dock_airplay_n.png";

    // 11. aicastPage2Button
    std::string aicast_path = _is_night_mode ?
        "/HomePage/dock_aicast_n_night.png" : "/HomePage/dock_aicast_n.png";

    // 12. miracastPage2Button
    std::string miracast_path = _is_night_mode ?
        "/HomePage/dock_miracast_n_night.png" : "/HomePage/dock_miracast_n.png";

    // 13. bluetoothPage2Button
    std::string bluetooth_path = _is_night_mode ?
        "/HomePage/dock_bt_n_night.png" : "/HomePage/dock_bt_n.png";
    if (mphonelinkPage2ButtonPtr) {
        mphonelinkPage2ButtonPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(bluetooth_path.c_str()).c_str());
    }

    std::string sound_progress_path = _is_night_mode ?
        "/HomePage/voice_progress_p_night.png" : "/HomePage/voice_progress_p.png";
    std::string sound_progress_bg_path = _is_night_mode ?
        "/HomePage/voice_progress_n_night.png" : "/HomePage/voice_progress_n.png";
    std::string icon_brightness_path = _is_night_mode ?
        "/HomePage/icon_brightness_n_night.png" : "/HomePage/icon_brightness_n.png";
    std::string icon_voice_path = _is_night_mode ?
            "/HomePage/icon_sound_n_night.png" : "/HomePage/icon_sound_n.png";
    if (mvoiceSeekBarPtr) {
        mvoiceSeekBarPtr->setProgressPic(CONFIGMANAGER->getResFilePath(sound_progress_path.c_str()).c_str());
        mSeekBar1Ptr->setProgressPic(CONFIGMANAGER->getResFilePath(sound_progress_path.c_str()).c_str());
        mvoiceTextViewPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(sound_progress_bg_path.c_str()).c_str());
        mbrightnessTextViewPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(sound_progress_bg_path.c_str()).c_str());
        miconVoiceTextViewPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(icon_voice_path.c_str()).c_str());
        miconBrightnessTextViewPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath(icon_brightness_path.c_str()).c_str());
    }

    std::string music_progress_bg_path = _is_night_mode ?
        "/HomePage/progress_n_night.png" : "/HomePage/progress_n.png";
    if (mPlayProgressSeekbarPtr) {
		mTextView2Ptr->setBackgroundPic(CONFIGMANAGER->getResFilePath(music_progress_bg_path.c_str()).c_str());
    }

    LOGD("[main] All backgrounds updated for %s mode", _is_night_mode ? "night" : "day");

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                          (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
    LOGD("[main] update_all_backgrounds_for_mode() EXIT, elapsed time: %.3f seconds", elapsed_time);
}

static void init_default_language() {
    std::string current_code = LANGUAGEMANAGER->getCurrentCode();

    if (current_code.empty() || current_code == "zh_CN") {  // 当前语言代码为空、中文或系统首次启动的话就设置为英文
        if (!FILE_EXIST("/data/.language_init_flag")) {
            LOGD("First boot detected, setting default language to English");
            sys::setting::update_localescode("en_US");
            system("touch /data/.language_init_flag");
            LOGD("Default language set to English (en_US)");
        } else {  // 如果标志文件存在但语言仍是中文,不强制覆盖
            LOGD("Language init flag exists, current language: %s", current_code.c_str());
        }
    } else {
        LOGD("Current language already set: %s", current_code.c_str());
    }
}

static void check_mcu_auto_upgrade() {
    LOGD("[MAIN] Checking for MCU auto upgrade...");
    
    if(FILE_EXIST("/mnt/extsd/mcuautoupgrade")){
		EASYUICONTEXT->openActivity("setfactoryActivity");
	}
}

static void _register_timer_fun(int id, int time) {
	mActivityPtr->registerUserTimer(id, time);
}

static void _unregister_timer_fun(int id) {
	mActivityPtr->unregisterUserTimer(id);
}

// 页面切换监听函数
static void onPageChange(int page) {
    _current_page_index = page;
    _is_in_sub_pages = (page != 0);
    LOGD("Page changed to: %d, is_in_sub_pages: %s", page, _is_in_sub_pages ? "true" : "false");
}

/**
 * Window7 已删除，不再需要页面切换
 */
static void _switch_app_window() {
	// Window7 removed - no-op
}

static void entry_lylink_ftu() {
	if (!sys::reverse_does_enter_status()) {
		EASYUICONTEXT->openActivity("lylinkviewActivity");
		_s_need_reopen_linkview = false;
	} else {
		LOGD("Is reverse status !!!\n");
		lk::video_stop();
		_s_need_reopen_linkview = true;
	}
}

// 更新CarPlay和AndroidAuto连接状态文本
static void update_link_status_text() {
	LYLINK_TYPE_E link_type = lk::get_lylink_type();

	// 更新CarPlay状态
	if (mcarplayPage2ButtonPtr) {
		if ((link_type == LINK_TYPE_WIFICP) || (link_type == LINK_TYPE_USBCP)) {
			mcarplayPage2ButtonPtr->setText("Connected");
		} else {
			mcarplayPage2ButtonPtr->setText("Not connected");
		}
	}

	// 更新AndroidAuto状态
	if (mandroidautoPage2ButtonPtr) {
		if ((link_type == LINK_TYPE_WIFIAUTO) || (link_type == LINK_TYPE_USBAUTO)) {
			mandroidautoPage2ButtonPtr->setText("Connected");
		} else {
			mandroidautoPage2ButtonPtr->setText("Not connected");
		}
	}
}

static void _lylink_callback(LYLINKAPI_EVENT evt, int para0, void *para1) {
	switch (evt) {
	case LYLINK_LINK_ESTABLISH:
		LOGD("LYLINK_LINK_ESTABLISH %s", lk::_link_type_to_str((LYLINK_TYPE_E) para0));
		EASYUICONTEXT->hideStatusBar();
		if (LINK_TYPE_AIRPLAY == para0 || LINK_TYPE_MIRACAST == para0 || LINK_TYPE_WIFILY == para0 || LINK_TYPE_WIFICP == para0) {
			if (bt::is_on()) {
				bt::power_off();
			}
			entry_lylink_ftu();
		}
		// BUG FIX: Android Auto 连接建立时也需要进入投屏界面
		// 与 CarPlay (WIFICP) 不同，AA 不需要关闭蓝牙（AA 依赖 BT 做配对/通话）
		if (LINK_TYPE_WIFIAUTO == para0 || LINK_TYPE_USBAUTO == para0) {
			entry_lylink_ftu();
		}
		if(sys::setting::get_sound_mode() == E_SOUND_MODE_LINK){
			if(media::music_is_playing()){
				media::music_pause();
			}
		}
		// 更新连接状态文本
		update_link_status_text();
		break;
	case LYLINK_LINK_DISCONN:
		LOGD("LYLINK_LINK_DISCONN........... %s", lk::_link_type_to_str((LYLINK_TYPE_E) para0));
		if (LINK_TYPE_AIRPLAY == para0 || LINK_TYPE_MIRACAST == para0 || LINK_TYPE_WIFILY == para0 || LINK_TYPE_WIFICP == para0) {
			if (!bt::is_on()) {
				bt::power_on();
			}
		}
		// BUG FIX: Android Auto 断连后也需要查询蓝牙状态并更新 UI
		// AA 断连时蓝牙本应是开着的（AA 需要 BT），但仍需确保状态正确
		if (LINK_TYPE_WIFIAUTO == para0 || LINK_TYPE_USBAUTO == para0) {
			if (!bt::is_on()) {
				bt::power_on();
			}
		}
		bt::query_state();
		EASYUICONTEXT->closeActivity("lylinkviewActivity");
		// 更新连接状态文本
		update_link_status_text();
		break;
	case LYLINK_PHONE_CONNECT:
		LOGD("LYLINK_PHONE_CONNECT %s", lk::_link_type_to_str((LYLINK_TYPE_E) para0));
		if (para0 == LINK_TYPE_WIFIAUTO || para0 == LINK_TYPE_WIFICP) {
			LOGD("You should open AP now.");
		}
		break;
	case LYLINK_FOREGROUND:
		LOGD("LYLINK_FOREGROUND");
		entry_lylink_ftu();
		break;
	case LYLINK_BACKGROUND:
	case LYLINK_HID_COMMAND:{
		if (evt == LYLINK_BACKGROUND) {
			LOGD("[main] LYLINK_BACKGROUND\n");
		} else {
			LOGD("[main] LYLINK_HID_COMMAND");
		}

		const char *app = EASYUICONTEXT->currentAppName();
		if (app && (strcmp(app, "lylinkviewActivity") == 0)) {
			EASYUICONTEXT->goHome();
		} else {
			EASYUICONTEXT->closeActivity("lylinkviewActivity");
		}
		_s_need_reopen_linkview = false;
	}
		break;
	case LYLINK_PHONE_DISCONN:
		LOGD("LYLINK_PHONE_DISCONN............. %s", lk::_link_type_to_str((LYLINK_TYPE_E) para0));
		lylinkapi_gocsdk("IA\r\n", strlen("IA\r\n"));
		// 更新连接状态文本
		update_link_status_text();
		break;
	default:
		break;
	}
}

static void _reverse_status_cb(int status) {
	LOGD("reverse status %d\n", status);
	base::runInUiThreadUniqueDelayed("rear_view_detection", [](){
		int status = sys::reverse_does_enter_status();
		LOGD("[main] reverse_status %d\n", status);
		if (status == REVERSE_STATUS_ENTER) {
			EASYUICONTEXT->openActivity("reverseActivity");
		} else {
			EASYUICONTEXT->closeActivity("reverseActivity");
			if (_s_need_reopen_linkview) {
				_s_need_reopen_linkview = false;
				if (lk::is_connected()) {
					EASYUICONTEXT->openActivity("lylinkviewActivity");
				}
			}
		}

	}, 50);
}

// parser() 已移除 — 主界面不再显示本地音乐信息，本地音乐可在后台播放

// 更新主界面音乐时间（蓝牙音乐）
static void update_main_music_time() {
    bt_music_t music_info = bt::get_music_info();
    int curPos = music_info.curpos;
    int maxPos = music_info.duration;

    static int last_valid_curPos = -1;
    static int last_valid_maxPos = -1;

    if (curPos >= 0) {
        last_valid_curPos = curPos;
    } else if (last_valid_curPos >= 0) {
        curPos = last_valid_curPos;
    }

    if (maxPos >= 0) {
        last_valid_maxPos = maxPos;
    } else if (last_valid_maxPos >= 0) {
        maxPos = last_valid_maxPos;
    }
}

static void _update_music_info() {
	bt_music_t music_info = bt::get_music_info();
	if (mtitleTextViewPtr) {
		mtitleTextViewPtr->setText(music_info.title);
	}
	if (martistTextViewPtr) {
		martistTextViewPtr->setText(music_info.artist);
		martistTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
	}
}

static void _update_music_progress() {
	bt_music_t music_info = bt::get_music_info();

	if (mPlayProgressSeekbarPtr) {
		mPlayProgressSeekbarPtr->setMax(music_info.duration);
		mPlayProgressSeekbarPtr->setProgress(music_info.curpos);
	}
}

static void _bt_music_cb(bt_music_state_e state) {
	if (bt::music_is_playing()) {
		_update_music_info();
		_update_music_progress();
		if (mtitleTextViewPtr) {
			mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
			mtitleTextViewPtr->setTextColor(0xFFFFFFFF);
		}
		if (mButtonPlayPtr) {
			mButtonPlayPtr->setSelected(true);
		}
		sys::setting::set_music_play_dev(E_AUDIO_TYPE_BT_MUSIC);
		update_main_music_time();
	} else {
		if (mtitleTextViewPtr) {
			mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
			mtitleTextViewPtr->setTextColor(0xFFFFFFFF);
		}
		if (mButtonPlayPtr) {
			mButtonPlayPtr->setSelected(false);
		}
	}
}

static void _music_play_status_cb(music_play_status_e status) {
	// 本地音乐不再驱动主界面显示，只保留后台播放逻辑
	switch (status) {
	case E_MUSIC_PLAY_STATUS_STARTED:
		LOGD("[main] local music started (background)");
		break;
	case E_MUSIC_PLAY_STATUS_RESUME:
		LOGD("[main] local music resumed (background)");
		break;
	case E_MUSIC_PLAY_STATUS_STOP:
		LOGD("[main] local music stopped (background)");
		break;
	case E_MUSIC_PLAY_STATUS_PAUSE:
		LOGD("[main] local music paused (background)");
		break;
	case E_MUSIC_PLAY_STATUS_COMPLETED:
		LOGD("[main] local music completed, play next (background)");
		media::music_next();
		break;
	case E_MUSIC_PLAY_STATUS_ERROR:
		LOGD("[main] local music error (background)");
		break;
	}
}

static void _bt_call_cb(bt_call_state_e state) {
	if (state != E_BT_CALL_STATE_IDLE) {
		if (lk::get_lylink_type() == LINK_TYPE_WIFIAUTO) {
			const char *app = EASYUICONTEXT->currentAppName();
			if (!app) return;
			if(strcmp(app, "reverseActivity") == 0) {
				_s_need_reopen_linkview = true;
			} else if(strcmp(app, "lylinkviewActivity") != 0) {
				EASYUICONTEXT->openActivity("lylinkviewActivity");
			}
		}
	}
}

static void _bt_add_cb() {
	_s_bt_cb.call_cb = _bt_call_cb;
	_s_bt_cb.music_cb = _bt_music_cb;
	bt::add_cb(&_s_bt_cb);
}

static void _bt_remove_cb() {
	bt::remove_cb(&_s_bt_cb);
}

static bool _show_sys_info(unsigned long *freeram) {
	struct sysinfo info;
	int ret = 0;
	ret = sysinfo(&info);
	if(ret != 0) {
		return false;
	}
	*freeram = info.freeram;
	return true;
}

static bool is_system_time_valid() {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    return (t->tm_year + 1900 >= 2020) || FILE_EXIST(TIME_SYNCED_FLAG);
}

static void ctrl_UI_init() {
	EASYUICONTEXT->hideStatusBar();
}

static void set_back_pic() {
//	if (mToMusicPtr) {
//		mToMusicPtr->setBackgroundPic(CONFIGMANAGER->getResFilePath("/HomePage/icon_media_cover_bg_n.png").c_str());
//	}
}

static void _preload_resources() {
	const char *pic_tab[] = {
		"/res/font/sanswithfz_jp_it_pt.ttf",
		"navi/bg_bt_n.png",
		"navi/bg_bt_p.png",
		"navi/bg_eq_n.png",
		"navi/bg_eq_p.png",
		"navi/bg_fm_n.png",
		"navi/bg_fm_p.png",
		"navi/bg_screen_off_n.png",
		"navi/bg_screen_off_p.png",
		"navi/icon_btvoice.png",
		"navi/icon_light.png",
		"navi/icon_setting_n.png",
		"navi/icon_setting_p.png",
		"navi/icon_voice.png",
		"navi/progress_n.png",
		"navi/progress_p.png",
	};

	LOGD("[main] preload resources start\n");

	size_t size = TAB_SIZE(pic_tab);
	for (size_t i = 0; i < size; ++i) {
		if (i == 0) {
			fy::cache_file(pic_tab[i]);
		} else {
			fy::cache_file(CONFIGMANAGER->getResFilePath(pic_tab[i]));
		}
	}

	LOGD("[main] preload resources end\n");
}

static void key_status(bool down) {
	static bool is_down = false;
	static uint32_t mDownTime;
	mActivityPtr->unregisterUserTimer(TIMER_POWERKEY_EVENT);
	if (down) {
		if (!is_down) {
			is_down = true;
			mActivityPtr->registerUserTimer(TIMER_POWERKEY_OFF, 1000);
			return ;
		}
	} else {
		if (is_down) {
			if (sys::reverse_does_enter_status()) {
				LOGD("[main] is reverse status, don't proc screensaver\n");
				return ;
			}
			mActivityPtr->registerUserTimer(TIMER_POWERKEY_EVENT, 100);
		}
		is_down = false;
		mActivityPtr->unregisterUserTimer(TIMER_POWERKEY_OFF);
		key_sec = 0;
	}
}

static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	{1,  1000},
	{QUERY_LINK_AUTH_TIMER, 6000},
	{SWITCH_ADB_TIMER, 1000},
};

static void onUI_init() {
	_preload_resources();
	ctrl_UI_init();

    _is_in_reverse_mode = false;
    _is_ui_update_paused = false;
	// local music cache removed
	// local music cache removed
    _is_exiting_reverse = false;
	// local music cache removed
	// local music cache removed
	// local music cache removed

    // 初始化白天/黑夜模式
    init_day_night_mode();

	mWindow6Ptr->showWnd();
 // [REMOVED] mWindow7Ptr->hideWnd();
 // [REMOVED] mStatusRadioGroupPtr->setCheckedID(ID_MAIN_RadioButton0);

	sys::setting::init();
	sys::hw::init();

	uart::init();
	uart::set_amplifier_mute(0);
	uart::set_power_cr(1);
	uart::add_power_state_cb(key_status);

	bt::init();
	mActivityPtr->registerUserTimer(BT_MUSIC_ID3, 0);

	net::init();

	media::init();
	media::music_add_play_status_cb(_music_play_status_cb);

	lk::add_lylink_callback(_lylink_callback);
	lk::start_lylink();

	app::attach_timer(_register_timer_fun, _unregister_timer_fun);

	sys::reverse_add_status_cb(_reverse_status_cb);
	sys::reverse_detect_start();

	_bt_add_cb();
	bt::query_state();

	media::music_add_play_status_cb(_music_play_status_cb);
	if (martistTextViewPtr) {
		martistTextViewPtr->setTouchPass(true);
	}
	if(bt::is_calling()){
		bt::call_vol(audio::get_lylink_call_vol());
	}

	base::UiHandler::implementTimerRegistration([]() {
		mActivityPtr->registerUserTimer(base::TIMER_UI_HANDLER, 0);
	});

	// 设置SeekBar加载标志
	main_seekbar_isLoad = true;

	// 初始化亮度SeekBar并注册监听器
	if (mSeekBar1Ptr) {
		LOGD("GANNINA1");
		int brightness = sys::setting::get_brightness();
		mSeekBar1Ptr->setProgress(brightness / 10);
	}

	// 初始化音量SeekBar
	if (mvoiceSeekBarPtr) {
		float current_vol = audio::get_system_vol();
		int current_step = (int)(current_vol * 10);
		mvoiceSeekBarPtr->setProgress(current_step);
	}
}

static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
    }
	mWindow6Ptr->showWnd();
 // [REMOVED] mWindow7Ptr->hideWnd();
 // [REMOVED] mStatusRadioGroupPtr->setCheckedID(ID_MAIN_RadioButton0);
}

static void onUI_show() {
	mode::set_switch_mode(E_SWITCH_MODE_NULL);
    LOGD("[main] Transitioning setup gannina");
    _is_ui_update_paused = false;
    _is_in_reverse_mode = false;

	int curPos = -1;
//    set_back_pic();

    // 更新所有背景(包括主背景和所有控件)
    update_all_backgrounds_for_mode();

    // 更新enableButton的选中状态
    if (menableButtonPtr) {
        menableButtonPtr->setSelected(_is_night_mode);
    }

	// musicWindow 全部跟蓝牙音乐连接
	_update_music_info();
	_update_music_progress();
	if (bt::music_is_playing()) {
		if (mButtonPlayPtr) {
			mButtonPlayPtr->setSelected(true);
		}
	} else {
		if (mButtonPlayPtr) {
			mButtonPlayPtr->setSelected(false);
		}
	}
	update_main_music_time();
    if (curPos >= 0) {
    	if (mPlayProgressSeekbarPtr) {
    		mPlayProgressSeekbarPtr->setProgress(curPos);
    	}
    }
	if (!app::is_show_topbar()) {
		sys::setting::set_reverse_topbar_show(true);
		app::show_topbar();
	}
	init_default_language();
	check_mcu_auto_upgrade();
	
	if (mmusictextPtr) {
		mmusictextPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
	}

	// 更新CarPlay和AndroidAuto连接状态
	update_link_status_text();

	// 更新亮度SeekBar的进度
	if (mSeekBar1Ptr) {
		int brightness = sys::setting::get_brightness();
		mSeekBar1Ptr->setProgress(brightness / 10);
	}

	base::runInUiThreadUniqueDelayed("voice_detection", [](){
		// 更新音量SeekBar的进度
		if (mvoiceSeekBarPtr) {
			if (sys::setting::get_fm_switch()) {
				mvoiceSeekBarPtr->setTouchable(false);
				mvoiceSeekBarPtr->setProgress(mvoiceSeekBarPtr->getMax());
			} else {
			    LOGD("[gannina] audio::get_system_vol voiceSeekBar %d !!!\n", audio::get_system_vol());
				mvoiceSeekBarPtr->setTouchable(true);
				float current_vol = audio::get_system_vol();
				int current_step = (int)(current_vol * 10);
				mvoiceSeekBarPtr->setProgress(current_step);
			}
		}

	}, 100);
}

static void onUI_hide() {
	struct timespec start_time, end_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	LOGD("[main] onUI_hide() ENTER");

	LOGD("[main] onUI_hide - cleaning up resources");
	_is_ui_update_paused = true;

	if (mTextViewBgPtr) {
		mTextViewBgPtr->setBackgroundBmp(NULL);
	}
	if (mmusicTextViewWindowPtr) {
		mmusicTextViewWindowPtr->setBackgroundPic(NULL);
	}
	if (mToMusicPtr) {
		mToMusicPtr->setBackgroundPic(NULL);
	}
	mmusicTextViewWindowPtr->setBackgroundPic(NULL);
	mandroidautoPage2ButtonPtr->setBackgroundPic(NULL);
	mcarplayPage2ButtonPtr->setBackgroundPic(NULL);
	maudiooutputButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] mmusicPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] mvideoPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] malbumPage2ButtonPtr->setBackgroundPic(NULL);
//	msettingPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] mairplayPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] maicastPage2ButtonPtr->setBackgroundPic(NULL);
	mphonelinkPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] mmiracastPage2ButtonPtr->setBackgroundPic(NULL);

	mvoiceSeekBarPtr->setProgressPic(NULL);
	mSeekBar1Ptr->setProgressPic(NULL);
	mvoiceTextViewPtr->setBackgroundPic(NULL);
	mbrightnessTextViewPtr->setBackgroundPic(NULL);
	miconVoiceTextViewPtr->setBackgroundPic(NULL);
	miconBrightnessTextViewPtr->setBackgroundPic(NULL);


	mnaviBgPtr->setBackgroundPic(NULL);
	mleftTFCardButtonPtr->setBackgroundPic(NULL);
	mleftReverseSettingButtonPtr->setBackgroundPic(NULL);
	mleftAudioButtonPtr->setBackgroundPic(NULL);
	mleftSetButtonPtr->setBackgroundPic(NULL);

	// local music cache removed
	// local music cache removed
	// local music cache removed

	clock_gettime(CLOCK_MONOTONIC, &end_time);
	double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
	                      (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	LOGD("[main] onUI_hide() EXIT, elapsed time: %.3f seconds", elapsed_time);
}

static void onUI_quit() {
	// 清除SeekBar加载标志
	main_seekbar_isLoad = false;

	uart::remove_power_state_cb(key_status);
	lk::remove_lylink_callback(_lylink_callback);
	media::music_remove_play_status_cb(_music_play_status_cb);
	if (mPlayProgressSeekbarPtr) {
		mPlayProgressSeekbarPtr->setSeekBarChangeListener(NULL);
	}

	if (mTextViewBgPtr) {
		mTextViewBgPtr->setBackgroundBmp(NULL);
	}
	if (mmusicTextViewWindowPtr) {
		mmusicTextViewWindowPtr->setBackgroundPic(NULL);
	}
	if (mToMusicPtr) {
		mToMusicPtr->setBackgroundPic(NULL);
	}
	mmusicTextViewWindowPtr->setBackgroundPic(NULL);
	mandroidautoPage2ButtonPtr->setBackgroundPic(NULL);
	mcarplayPage2ButtonPtr->setBackgroundPic(NULL);
	maudiooutputButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] mmusicPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] mvideoPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] malbumPage2ButtonPtr->setBackgroundPic(NULL);
//	msettingPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] mairplayPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] maicastPage2ButtonPtr->setBackgroundPic(NULL);
	mphonelinkPage2ButtonPtr->setBackgroundPic(NULL);
 // [REMOVED] mmiracastPage2ButtonPtr->setBackgroundPic(NULL);

	mvoiceSeekBarPtr->setProgressPic(NULL);
	mSeekBar1Ptr->setProgressPic(NULL);
	mvoiceTextViewPtr->setBackgroundPic(NULL);
	mbrightnessTextViewPtr->setBackgroundPic(NULL);
	miconVoiceTextViewPtr->setBackgroundPic(NULL);
	miconBrightnessTextViewPtr->setBackgroundPic(NULL);

	mnaviBgPtr->setBackgroundPic(NULL);
	mleftTFCardButtonPtr->setBackgroundPic(NULL);
	mleftReverseSettingButtonPtr->setBackgroundPic(NULL);
	mleftAudioButtonPtr->setBackgroundPic(NULL);
	mleftSetButtonPtr->setBackgroundPic(NULL);
	// local music cache removed
	// local music cache removed
	// local music cache removed
	// local music cache removed

	_bt_remove_cb();
}

static bool onUI_back() {
	_switch_app_window();
	return false;
}

static void onProtocolDataUpdate(const SProtocolData &data) {

}

static bool onUI_Timer(int id) {
	if (app::on_timer(id)) {
		return false;
	}
	switch (id) {
	case 0: {
		unsigned long freeram = 0;
		bool ret = _show_sys_info(&freeram);
		if(ret) {
			LOGD("-----------Current MemFree: %ldKB---------------", freeram >> 10);
		} else {
			LOGD("-----------get MemFree info fail----------------");
		}
	}
		break;
	case 1: {
        // musicWindow 全部跟蓝牙音乐连接
        if (bt::music_is_playing()) {
            _update_music_progress();
        }
        update_main_music_time();
	}
		break;
	case QUERY_LINK_AUTH_TIMER:
		lk::query_is_authorized();
		return false;
	case SWITCH_ADB_TIMER: {
		if (strcmp("UpgradeActivity", EASYUICONTEXT->currentAppName()) == 0) {
			app::hide_topbar();
		}
		if (sys::setting::is_usb_adb_enabled()) {
			if (sys::get_usb_mode() != E_USB_MODE_DEVICE) {
				sys::change_usb_mode(E_USB_MODE_DEVICE);
			}
		} else {
			sys::set_usb_config(E_USB_MODE_HOST);
		}
		uart::set_amplifier_mute(1);
	}
		return false;
	case BT_MUSIC_ID3:
		if (bt::music_is_playing()) {
			bt::query_music_info();
		}
		return false;
	case MUSIC_ERROR_TIMER:
		media::music_next(true);
		return false;
    case base::TIMER_UI_HANDLER:
      return base::UiHandler::onTimer();
      break;
	case TIMER_POWERKEY_EVENT:
		if (strcmp("screenOffActivity", EASYUICONTEXT->currentAppName()) == 0) {
			EASYUICONTEXT->closeActivity("screenOffActivity");
		} else {
			fold_statusbar();
	    	EASYUICONTEXT->openActivity("screenOffActivity");
		}
		return false;
	case TIMER_POWERKEY_OFF:{
		if ((key_sec++) >= 2) {
			if (sys::reverse_does_enter_status()) {
				LOGD("[main] is reverse status, don't proc key long press\n");
				key_sec = 0;
				return false;
			}
			if (lk::is_connected()) {
				lk::stop_lylink();
			}
			uart::app_turn_off();
		}
	}
		break;
	default:
		break;
	}
    return true;
}

/**
 * 检查触摸点是否在SeekBar区域内
 * 用于防止在操作SeekBar时误触发窗口切换
 */
static bool isTouchInSeekBarArea(int x, int y) {
	int margin = 20;  // 扩大检测区域，增加容错范围（上下左右各扩展20像素）

	// 检查音量SeekBar区域
	if (mvoiceSeekBarPtr && mvoiceSeekBarPtr->isVisible()) {
		LayoutPosition voicePos = mvoiceSeekBarPtr->getPosition();
		if (x >= (voicePos.mLeft - margin) && x <= (voicePos.mLeft + voicePos.mWidth + margin) &&
		    y >= (voicePos.mTop - margin) && y <= (voicePos.mTop + voicePos.mHeight + margin)) {
			return true;
		}
	}

	// 检查亮度SeekBar区域
	if (mSeekBar1Ptr && mSeekBar1Ptr->isVisible()) {
		LayoutPosition brightPos = mSeekBar1Ptr->getPosition();
		if (x >= (brightPos.mLeft - margin) && x <= (brightPos.mLeft + brightPos.mWidth + margin) &&
		    y >= (brightPos.mTop - margin) && y <= (brightPos.mTop + brightPos.mHeight + margin)) {
			return true;
		}
	}

	// 检查播放进度SeekBar区域
	if (mPlayProgressSeekbarPtr && mPlayProgressSeekbarPtr->isVisible()) {
		LayoutPosition playPos = mPlayProgressSeekbarPtr->getPosition();
		if (x >= (playPos.mLeft - margin) && x <= (playPos.mLeft + playPos.mWidth + margin) &&
		    y >= (playPos.mTop - margin) && y <= (playPos.mTop + playPos.mHeight + margin)) {
			return true;
		}
	}

	// 检查ctrlbar是否显示，如果显示则检查触摸点是否在ctrlbar区域内
	// ctrlbar包含音量和亮度SeekBar，当它显示时禁止主界面的滑动切换
	if (app::is_show_ctrlbar()) {
		return true;
	}

	return false;
}

static bool onmainActivityTouchEvent(const MotionEvent &ev) {
	LayoutPosition pos = EASYUICONTEXT->getNaviBar()->getPosition();

	static MotionEvent down_ev;
	static bool allow_switch;
	static bool is_seekbar_touch;  // 标记是否在SeekBar区域触摸

	if (pos.mTop != -pos.mHeight) {	return false; }
	switch (ev.mActionStatus) {
	case MotionEvent::E_ACTION_DOWN:
		// 检查触摸起始点是否在SeekBar区域
		is_seekbar_touch = isTouchInSeekBarArea(ev.mX, ev.mY);
		if (is_seekbar_touch) {
			// 如果在SeekBar区域，不允许切换窗口
			allow_switch = false;
			LOGD("[main] Touch started in SeekBar area, disable window switch");
		} else {
			allow_switch = true;
		}
		down_ev = ev;
		break;
	case MotionEvent::E_ACTION_MOVE:
		// 如果正在SeekBar区域操作，继续禁止窗口切换
		if (is_seekbar_touch) {
			allow_switch = false;
		}
		break;
	case MotionEvent::E_ACTION_UP:
	    // Window7 removed - swipe to switch pages disabled
	    allow_switch = false;
	    is_seekbar_touch = false;
	    break;
	default:
		break;
	}
	return false;
}

static bool onButtonClick_NextButton(ZKButton *pButton) {
    LOGD(" ButtonClick NextButton !!!\n");
	if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}
	bt::music_next();
    return false;
}

static bool onButtonClick_ButtonPlay(ZKButton *pButton) {
    LOGD(" ButtonClick ButtonPlay !!!\n");

	if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}

	bt::music_is_playing() ? bt::music_pause() : bt::music_play();
    return false;
}

static bool onButtonClick_PrevButton(ZKButton *pButton) {
    LOGD(" ButtonClick PrevButton !!!\n");
	if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}
	bt::music_prev();
    return false;
}

static void onProgressChanged_PlayProgressSeekbar(ZKSeekBar *pSeekBar, int progress) {
}

static bool onButtonClick_Setting(ZKButton *pButton) {
    LOGD(" ButtonClick Setting !!!\n");
    EASYUICONTEXT->openActivity("settingsActivity");
    return false;
}

static bool onButtonClick_ToMusic(ZKButton *pButton) {
    LOGD(" ButtonClick ToMusic → entering btsettingActivity (BT music)\n");
	if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}
	EASYUICONTEXT->openActivity("btsettingActivity");
    return false;
}

static void open_linkhelp_activity(link_mode_e mode) {
	Intent *i = new Intent;
	i->putExtra("link_mode", fy::format("%d", mode));
	LOGD("[main] choose link mode %s\n", sys::setting::get_link_mode_str(mode));
	EASYUICONTEXT->openActivity("linkhelpActivity", i);
}

static void _change_link_app(link_mode_e mode) {
	switch (mode) {
	case E_LINK_MODE_HICAR:
	case E_LINK_MODE_ANDROIDAUTO:
	case E_LINK_MODE_CARPLAY:
		if (net::get_mode() != E_NET_MODE_AP) { net::change_mode(E_NET_MODE_AP);}
		break;
	case E_LINK_MODE_AIRPLAY:
		if (net::get_mode() != E_NET_MODE_AP) { net::change_mode(E_NET_MODE_AP); }
		break;
	case E_LINK_MODE_CARLIFE:
		if (net::get_mode() != E_NET_MODE_WIFI) { net::change_mode(E_NET_MODE_WIFI); }
		break;
	case E_LINK_MODE_MIRACAST:
	case E_LINK_MODE_LYLINK:
		if (net::get_mode() != E_NET_MODE_P2P) { net::change_mode(E_NET_MODE_P2P); }
		break;
	default:
		break;
	}
	open_linkhelp_activity(mode);
}

static void open_link_activity(link_mode_e mode) {
	LYLINK_TYPE_E link_type = lk::get_lylink_type();
	switch(mode) {
	case E_LINK_MODE_CARPLAY:
		if ((link_type == LINK_TYPE_WIFICP) || (link_type == LINK_TYPE_USBCP)) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_ANDROIDAUTO:
		if ((link_type == LINK_TYPE_WIFIAUTO) || (link_type == LINK_TYPE_USBAUTO)) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_CARLIFE:
		if ((link_type == LINK_TYPE_WIFILIFE) || (link_type == LINK_TYPE_USBLIFE)) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_HICAR:
		if ((link_type == LINK_TYPE_WIFIHICAR) || (link_type == LINK_TYPE_USBHICAR)) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_MIRACAST:
		if (link_type == LINK_TYPE_MIRACAST) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_LYLINK:
		if (link_type == LINK_TYPE_WIFILY) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_AIRPLAY:
		if (link_type == LINK_TYPE_AIRPLAY) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	default:
		break;
	}
	if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return;
	}

	if (mode == E_LINK_MODE_AIRPLAY || mode == E_LINK_MODE_LYLINK || mode ==E_LINK_MODE_MIRACAST) {
		if (bt::is_on()) {
			bt::power_off();
		}
	} else {
		if (!bt::is_on()) {
			bt::power_on();
		}
	}

	open_linkhelp_activity(mode);
}

static bool onButtonClick_Button1(ZKButton *pButton) {
    LOGD(" ButtonClick Button1 !!!\n");
    EASYUICONTEXT->openActivity("mcubtUpdActivity");
    return false;
}

static bool onButtonClick_Button2(ZKButton *pButton) {
    LOGD(" ButtonClick Button2 !!!\n");
    EASYUICONTEXT->openActivity("soundEffectActivity");
    return false;
}

static void onCheckedChanged_StatusRadioGroup(ZKRadioGroup* pRadioGroup, int checkedID) {
    // Window7 removed - radio group no longer toggles pages
    LOGD("[main] StatusRadioGroup checked %d (Window7 removed, no-op)", checkedID);
}

static void onProgressChanged_PlayVolSeekBar(ZKSeekBar *pSeekBar, int progress) {
}

static void onProgressChanged_voiceSeekBar(ZKSeekBar *pSeekBar, int progress) {
    LOGD("[gannina] ProgressChanged voiceSeekBar %d !!!\n", progress);
	if (sys::setting::get_fm_switch()) {
		return;  // FM模式下不调节音量
	}
	bool effect = bt::is_calling() || (lk::is_connected() && lk::get_is_call_state() != CallState_Hang);
	audio::set_system_vol(progress / 10.f, !effect);
	// 同步到其他界面
	set_navibar_PlayVolSeekBar(progress);
	set_ctrlbar_volumSeekBar(progress);
	setSettingFtu_MediaSeekBar(progress);
}

static bool onButtonClick_button_apps(ZKButton *pButton) {
    LOGD(" ButtonClick button_apps !!!\n");
    return false;
}

static void onCheckedChanged_RadioGroup1(ZKRadioGroup* pRadioGroup, int checkedID) {
    LOGD(" RadioGroup RadioGroup1 checked %d", checkedID);
}

static bool onButtonClick_audiooutputButton(ZKButton *pButton) {
    LOGD(" ButtonClick audiooutputButton !!!\n");
    EASYUICONTEXT->openActivity("FMemitActivity");
    return false;
}

static void onCheckedChanged_page3RadioGroup(ZKRadioGroup* pRadioGroup, int checkedID) {
    LOGD(" RadioGroup page3RadioGroup checked %d", checkedID);
}

static bool onButtonClick_carplayPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick carplayPage2Button !!!\n");
	open_link_activity(E_LINK_MODE_CARPLAY);
    return false;
}

static bool onButtonClick_androidautoPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick androidautoPage2Button !!!\n");
    open_link_activity(E_LINK_MODE_ANDROIDAUTO);
    return false;
}

static bool onButtonClick_airplayPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick airplayPage2Button !!!\n");
    open_link_activity(E_LINK_MODE_AIRPLAY);
    return false;
}

static bool onButtonClick_aicastPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick aicastPage2Button !!!\n");
    open_link_activity(E_LINK_MODE_LYLINK);
    return false;
}

static bool onButtonClick_miracastPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick miracastPage2Button !!!\n");
    open_link_activity(E_LINK_MODE_MIRACAST);
    return false;
}

static bool onButtonClick_phonelinkPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick phonelinkPage2Button !!!\n");
    // 如果已连接，直接进入投屏画面
    if (lk::is_connected()) {
        EASYUICONTEXT->openActivity("lylinkviewActivity");
        return false;
    }
    // 否则进入 phonelink 选择界面
    Intent *intent = new Intent();
    intent->putExtra("link_mode", fy::format("%d", sys::setting::get_link_mode()));
    EASYUICONTEXT->openActivity("phonelinkActivity", intent);
    return false;
}

static bool onButtonClick_musicPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick musicPage2Button !!!\n");
    if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}
	EASYUICONTEXT->openActivity("musicActivity");
    return false;
}

static bool onButtonClick_videoPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick videoPage2Button !!!\n");
    if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}
	EASYUICONTEXT->openActivity("videoActivity");
    return false;
}

static bool onButtonClick_albumPage2Button(ZKButton *pButton) {
    LOGD(" ButtonClick albumPage2Button !!!\n");
    if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}
	EASYUICONTEXT->openActivity("PhotoAlbumActivity");
    return false;
}

//static bool onButtonClick_settingPage2Button(ZKButton *pButton) {
//    LOGD(" ButtonClick settingPage2Button !!!\n");
//    EASYUICONTEXT->openActivity("settingsActivity");
//    return false;
//}

/**
 * enableButton 点击事件 - 控制白天/黑夜模式切换
 */
static bool onButtonClick_enableButton(ZKButton *pButton) {
    LOGD(" ButtonClick enableButton !!!\n");

    bool current_mode = pButton->isSelected();
    bool to_night_mode = !current_mode;

    // 1. 先设置模式状态和按钮
    set_night_mode(to_night_mode);
    pButton->setSelected(to_night_mode);

    // 2. 更新背景图片
    update_all_backgrounds_for_mode();

    // 3. 延迟50ms后再调亮度，确保图片渲染完成
    base::runInUiThreadUniqueDelayed("brightness_adjust", [to_night_mode](){
        if (mSeekBar1Ptr) {
            if (to_night_mode) {
                mSeekBar1Ptr->setProgress(1);
                sys::setting::set_brightness(10);
                set_navibar_brightnessBar(1);
                set_ctrlbar_lightSeekBar(1);
                setSettingFtu_BrillianceSeekBar(1);
                LOGD("[main] Night mode brightness set to 10%%");
            } else {
                mSeekBar1Ptr->setProgress(7);
                sys::setting::set_brightness(70);
                set_navibar_brightnessBar(7);
                set_ctrlbar_lightSeekBar(7);
                setSettingFtu_BrillianceSeekBar(7);
                LOGD("[main] Day mode brightness set to 70%%");
            }
        }
    }, 50);

    return false;
}

static bool onButtonClick_audiooutput3Button(ZKButton *pButton) {
    LOGD(" ButtonClick audiooutput3Button !!!\n");
    return false;
}
static bool onButtonClick_leftTFCardButton(ZKButton *pButton) {
    LOGD(" ButtonClick leftTFCardButton !!!\n");
    if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}
	EASYUICONTEXT->openActivity("tfcardActivity");
    return false;
}

static bool onButtonClick_leftReverseSettingButton(ZKButton *pButton) {
    LOGD(" ButtonClick leftReverseSettingButton !!!\n");
    // 倒车设置不受互联状态限制
	EASYUICONTEXT->openActivity("setreverseActivity");
    return false;
}

static bool onButtonClick_leftAudioButton(ZKButton *pButton) {
    LOGD(" ButtonClick leftAudioButton !!!\n");
    EASYUICONTEXT->openActivity("FMemitActivity");
    return false;
}

static bool onButtonClick_leftSetButton(ZKButton *pButton) {
    LOGD(" ButtonClick leftSetButton !!!\n");
    EASYUICONTEXT->openActivity("settingsActivity");
    return false;
}
static void onProgressChanged_SeekBar1(ZKSeekBar *pSeekBar, int progress) {
    LOGD("[gannina3] ProgressChanged SeekBar1 %d !!!\n", progress);
	sys::setting::set_brightness(progress * 10);
	// 同步到其他界面
	set_navibar_brightnessBar(progress);
	set_ctrlbar_lightSeekBar(progress);
	setSettingFtu_BrillianceSeekBar(progress);
}

static bool onButtonClick_toLocalmusicButton(ZKButton *pButton) {
    LOGD(" ButtonClick toLocalmusicButton → entering btsettingActivity (BT music)\n");
    if (lk::is_connected()) {
		if (mlinkTipsWindowPtr) {
			mlinkTipsWindowPtr->showWnd();
		}
		return false;
	}
	EASYUICONTEXT->openActivity("btsettingActivity");
    return false;
}
