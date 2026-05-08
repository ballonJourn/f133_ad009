#pragma once
#include "uart/ProtocolSender.h"

/*
 * tfcardLogic.cc
 * TF Card 媒体浏览器 —— 左侧文件夹列表 + 右侧文件列表 + 底部类型切换
 */

#include "media/media_context.h"
#include "media/music_player.h"
#include "system/setting.h"
#include "link/context.h"
#include <set>
#include <algorithm>

// ==================== 类型定义 ====================

typedef enum {
	E_NAV_MUSIC = 0,
	E_NAV_VIDEO,
	E_NAV_PHOTO,
} nav_type_e;

static const char* _nav_type_str(nav_type_e nav) {
	switch (nav) {
	case E_NAV_MUSIC: return "Music";
	case E_NAV_VIDEO: return "Video";
	case E_NAV_PHOTO: return "Photo";
	}
	return "Unknown";
}

// ==================== 状态变量 ====================

static nav_type_e _s_current_nav = E_NAV_MUSIC;
static int _s_selected_folder_index = -1;
static storage_type_e _s_storage = E_STORAGE_TYPE_SD;

static std::vector<std::string> _s_folder_list;
static std::vector<int> _s_filtered_file_indices;

// SD 卡是否挂载
static bool _s_sd_mounted = false;

#define SCAN_CHECK_TIMER   1
#define LOADING_TIMEOUT    2
#define NAV_SWITCH_TIMER   3

// ==================== 辅助函数 ====================

static std::string _get_folder_path(const std::string &file_path) {
	size_t pos = file_path.rfind('/');
	if (pos == std::string::npos || pos == 0) {
		return "/";
	}
	return file_path.substr(0, pos);
}

static std::string _get_folder_display_name(const std::string &folder_path) {
	size_t pos = folder_path.rfind('/');
	if (pos == std::string::npos || pos >= folder_path.size() - 1) {
		return folder_path;
	}
	return folder_path.substr(pos + 1);
}

static std::string _get_file_name(const std::string &file_path) {
	size_t pos = file_path.rfind('/');
	if (pos == std::string::npos) {
		return file_path;
	}
	return file_path.substr(pos + 1);
}

static int _get_total_file_count() {
	switch (_s_current_nav) {
	case E_NAV_MUSIC: return media::get_audio_list_size(_s_storage);
	case E_NAV_VIDEO: return media::get_video_list_size(_s_storage);
	case E_NAV_PHOTO: return media::get_image_list_size(_s_storage);
	}
	return 0;
}

static std::string _get_file_by_index(int index) {
	switch (_s_current_nav) {
	case E_NAV_MUSIC: return media::get_audio_file(_s_storage, index);
	case E_NAV_VIDEO: return media::get_video_file(_s_storage, index);
	case E_NAV_PHOTO: return media::get_image_file(_s_storage, index);
	}
	return "";
}

/**
 * 检测 SD 卡是否挂载（通过检查是否有任何类型的媒体文件）
 */
static bool _check_sd_mounted() {
	return (media::get_audio_list_size(E_STORAGE_TYPE_SD) > 0 ||
	        media::get_video_list_size(E_STORAGE_TYPE_SD) > 0 ||
	        media::get_image_list_size(E_STORAGE_TYPE_SD) > 0);
}

// ==================== 核心数据构建 ====================

static void _rebuild_folder_list() {
	_s_folder_list.clear();
	std::set<std::string> folder_set;

	int total = _get_total_file_count();
	LOGD("[tfcard] _rebuild_folder_list: nav=%s, total_files=%d",
		_nav_type_str(_s_current_nav), total);

	for (int i = 0; i < total; i++) {
		std::string file = _get_file_by_index(i);
		if (!file.empty()) {
			std::string folder = _get_folder_path(file);
			if (folder_set.find(folder) == folder_set.end()) {
				folder_set.insert(folder);
				_s_folder_list.push_back(folder);
			}
		}
	}

	std::sort(_s_folder_list.begin(), _s_folder_list.end());

	LOGD("[tfcard] folder list built: %d folders", (int)_s_folder_list.size());
	for (int i = 0; i < (int)_s_folder_list.size(); i++) {
		LOGD("[tfcard]   folder[%d]: %s → display: %s",
			i, _s_folder_list[i].c_str(),
			_get_folder_display_name(_s_folder_list[i]).c_str());
	}
}

static void _rebuild_file_list() {
	_s_filtered_file_indices.clear();

	if (_s_selected_folder_index < 0 ||
		_s_selected_folder_index >= (int)_s_folder_list.size()) {
		LOGD("[tfcard] _rebuild_file_list: no folder selected (index=%d, folders=%d)",
			_s_selected_folder_index, (int)_s_folder_list.size());
		return;
	}

	std::string target_folder = _s_folder_list[_s_selected_folder_index];
	int total = _get_total_file_count();

	for (int i = 0; i < total; i++) {
		std::string file = _get_file_by_index(i);
		if (!file.empty()) {
			std::string folder = _get_folder_path(file);
			if (folder == target_folder) {
				_s_filtered_file_indices.push_back(i);
			}
		}
	}

	LOGD("[tfcard] file list built: %d files in [%s]",
		(int)_s_filtered_file_indices.size(), target_folder.c_str());
}

// ==================== UI 刷新 ====================

static void _refresh_all() {
	LOGD("[tfcard] === _refresh_all START ===");

	_s_sd_mounted = _check_sd_mounted();
	LOGD("[tfcard] SD mounted: %s, nav: %s",
		_s_sd_mounted ? "YES" : "NO", _nav_type_str(_s_current_nav));

	_rebuild_folder_list();

	// 自动选中第一个文件夹
	if (!_s_folder_list.empty()) {
		if (_s_selected_folder_index < 0 ||
			_s_selected_folder_index >= (int)_s_folder_list.size()) {
			_s_selected_folder_index = 0;
		}
	} else {
		_s_selected_folder_index = -1;
	}

	_rebuild_file_list();

	LOGD("[tfcard] refreshing ListViews: folders=%d, files=%d, selected_folder=%d",
		(int)_s_folder_list.size(), (int)_s_filtered_file_indices.size(),
		_s_selected_folder_index);

	if (mfolderListViewPtr) {
		mfolderListViewPtr->refreshListView();
	}
	if (mListView1Ptr) {
		mListView1Ptr->refreshListView();
	}

	LOGD("[tfcard] === _refresh_all END ===");
}

static void _update_nav_selection() {
	LOGD("[tfcard] _update_nav_selection: current=%s", _nav_type_str(_s_current_nav));

	if (mnavMusicButtonPtr) {
		mnavMusicButtonPtr->setSelected(_s_current_nav == E_NAV_MUSIC);
		LOGD("[tfcard]   navMusicButton selected=%d", _s_current_nav == E_NAV_MUSIC);
	}
	if (mButton1Ptr) {
		mButton1Ptr->setSelected(_s_current_nav == E_NAV_VIDEO);
		LOGD("[tfcard]   Button1(Video) selected=%d", _s_current_nav == E_NAV_VIDEO);
	}
	if (mnavPhotoButtonPtr) {
		mnavPhotoButtonPtr->setSelected(_s_current_nav == E_NAV_PHOTO);
		LOGD("[tfcard]   navPhotoButton selected=%d", _s_current_nav == E_NAV_PHOTO);
	}
}

static void _show_loading() {
	LOGD("[tfcard] _show_loading");
	if (mloadingPopupWindowPtr) {
		mloadingPopupWindowPtr->showWnd();
	}
}

static void _hide_loading() {
	LOGD("[tfcard] _hide_loading");
	if (mloadingPopupWindowPtr) {
		mloadingPopupWindowPtr->hideWnd();
	}
}

static void _switch_nav(nav_type_e nav) {
	LOGD("[tfcard] _switch_nav: %s → %s",
		_nav_type_str(_s_current_nav), _nav_type_str(nav));

	if (_s_current_nav == nav) {
		LOGD("[tfcard]   same nav, skip");
		return;
	}

	_s_current_nav = nav;
	_s_selected_folder_index = -1;  // 切换类型时重置文件夹选择
	_update_nav_selection();

	// 显示loading
	_show_loading();

	// 延迟刷新，让 loading 有时间渲染
	mActivityPtr->registerUserTimer(NAV_SWITCH_TIMER, 150);
}

// ==================== 扫描回调 ====================

static void _media_scan_cb(const char *dir, storage_type_e type, bool started) {
	if (type != _s_storage) {
		return;
	}

	if (started) {
		LOGD("[tfcard] scan started: %s", dir);
		_show_loading();
	} else {
		LOGD("[tfcard] scan finished: %s", dir);
		_refresh_all();
		_hide_loading();
	}
}

// ==================== 生命周期 ====================

static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000},
};

static void onUI_init() {
	LOGD("[tfcard] ========== onUI_init ==========");
	_s_current_nav = E_NAV_MUSIC;
	_s_selected_folder_index = -1;
	_s_storage = E_STORAGE_TYPE_SD;
	_s_sd_mounted = false;
	_s_folder_list.clear();
	_s_filtered_file_indices.clear();

	media::add_scan_cb(_media_scan_cb);
	LOGD("[tfcard] scan callback registered");
}

static void onUI_intent(const Intent *intentPtr) {
	LOGD("[tfcard] onUI_intent");
	if (intentPtr != NULL) {
		std::string nav = intentPtr->getExtra("nav_type");
		LOGD("[tfcard]   nav_type from intent: [%s]", nav.c_str());
		if (nav == "video") {
			_s_current_nav = E_NAV_VIDEO;
		} else if (nav == "photo") {
			_s_current_nav = E_NAV_PHOTO;
		} else {
			_s_current_nav = E_NAV_MUSIC;
		}
	}
}

static void onUI_show() {
	LOGD("[tfcard] ========== onUI_show ==========");

	_update_nav_selection();

	// 打印完整的系统媒体状态
	LOGD("[tfcard] media status: audio=%d, video=%d, image=%d (SD)",
		media::get_audio_list_size(E_STORAGE_TYPE_SD),
		media::get_video_list_size(E_STORAGE_TYPE_SD),
		media::get_image_list_size(E_STORAGE_TYPE_SD));
	LOGD("[tfcard] media::is_scanning() = %d", media::is_scanning());

	if (media::is_scanning()) {
		LOGD("[tfcard] scan in progress, showing loading");
		_show_loading();
	} else {
		_s_sd_mounted = _check_sd_mounted();
		LOGD("[tfcard] scan idle, sd_mounted=%d", _s_sd_mounted);

		if (!_s_sd_mounted) {
			// SD 卡未挂载或无任何媒体文件
			LOGD("[tfcard] NO SD card or NO media files — show loading then timeout");
			_show_loading();
			mActivityPtr->registerUserTimer(LOADING_TIMEOUT, 2000);
		} else {
			// 有文件，直接刷新
			LOGD("[tfcard] SD has media, refreshing...");
			_refresh_all();
		}
	}

	if (mTextViewmusicbuttonPtr)  mTextViewmusicbuttonPtr->setTouchPass(true);
	if (mTextViewvideobuttonPtr)  mTextViewvideobuttonPtr->setTouchPass(true);
	if (mTextViewalbumbuttonPtr)  mTextViewalbumbuttonPtr->setTouchPass(true);
}

static void onUI_hide() {
	LOGD("[tfcard] onUI_hide");
}

static void onUI_quit() {
	LOGD("[tfcard] onUI_quit");
	media::remove_scan_cb(_media_scan_cb);
	_s_folder_list.clear();
	_s_filtered_file_indices.clear();
}

static void onProtocolDataUpdate(const SProtocolData &data) {
}

static bool onUI_Timer(int id) {
	switch (id) {
	case SCAN_CHECK_TIMER:
		LOGD("[tfcard] SCAN_CHECK_TIMER fired");
		_refresh_all();
		_hide_loading();
		return false;
	case LOADING_TIMEOUT:
		LOGD("[tfcard] LOADING_TIMEOUT fired — hiding loading, clearing lists");
		_hide_loading();
		// 确保列表显示 0 行而不是残留数据
		_s_folder_list.clear();
		_s_filtered_file_indices.clear();
		if (mfolderListViewPtr) mfolderListViewPtr->refreshListView();
		if (mListView1Ptr) mListView1Ptr->refreshListView();
		return false;
	case NAV_SWITCH_TIMER:
		LOGD("[tfcard] NAV_SWITCH_TIMER fired — refreshing for %s",
			_nav_type_str(_s_current_nav));
		_refresh_all();
		_hide_loading();
		return false;
	default:
		break;
	}
	return true;
}

static bool ontfcardActivityTouchEvent(const MotionEvent &ev) {
	switch (ev.mActionStatus) {
	case MotionEvent::E_ACTION_DOWN:
		break;
	case MotionEvent::E_ACTION_MOVE:
		break;
	case MotionEvent::E_ACTION_UP:
		break;
	default:
		break;
	}
	return false;
}

// ==================== 左侧文件夹列表回调 ====================

static int getListItemCount_folderListView(const ZKListView *pListView) {
	int count = (int)_s_folder_list.size();
	LOGD("[tfcard] getListItemCount_folderListView → %d", count);
	return count;
}


static void onListItemClick_folderListView(ZKListView *pListView, int index, int id) {
	LOGD("[tfcard] folder click: index=%d (total=%d)", index, (int)_s_folder_list.size());

	if (index < 0 || index >= (int)_s_folder_list.size()) {
		LOGD("[tfcard]   invalid index, ignoring");
		return;
	}

	_s_selected_folder_index = index;
	LOGD("[tfcard]   selected folder: %s",
		_get_folder_display_name(_s_folder_list[index]).c_str());

	_rebuild_file_list();

	if (mfolderListViewPtr) mfolderListViewPtr->refreshListView();
	if (mListView1Ptr) mListView1Ptr->refreshListView();
}

// ==================== 右侧文件列表回调 ====================

static int getListItemCount_ListView1(const ZKListView *pListView) {
	int count = (int)_s_filtered_file_indices.size();
	LOGD("[tfcard] getListItemCount_ListView1 → %d", count);
	return count;
}



static void onListItemClick_ListView1(ZKListView *pListView, int index, int id) {
	LOGD("[tfcard] file click: index=%d, subItem id=%d", index, id);

	if (index < 0 || index >= (int)_s_filtered_file_indices.size()) {
		LOGD("[tfcard]   invalid index, ignoring");
		return;
	}

	int real_index = _s_filtered_file_indices[index];
	std::string file = _get_file_by_index(real_index);
	LOGD("[tfcard]   file: %s (real_index=%d)", file.c_str(), real_index);

	if (lk::is_connected()) {
		LOGD("[tfcard]   link connected, ignoring file click");
		return;
	}

	switch (_s_current_nav) {
	case E_NAV_MUSIC: {
		LOGD("[tfcard]   → opening musicActivity, play index=%d", real_index);
		sys::setting::set_music_play_dev(E_AUDIO_TYPE_MUSIC);
		media::music_play(_s_storage, real_index);
		EASYUICONTEXT->openActivity("musicActivity");
		break;
	}
	case E_NAV_VIDEO: {
		LOGD("[tfcard]   → opening videoActivity");
		EASYUICONTEXT->openActivity("videoActivity");
		break;
	}
	case E_NAV_PHOTO: {
		LOGD("[tfcard]   → opening PhotoAlbumActivity");
		EASYUICONTEXT->openActivity("PhotoAlbumActivity");
		break;
	}
	}
}

// ==================== 底部导航按钮 ====================

static bool onButtonClick_navMusicButton(ZKButton *pButton) {
	LOGD("[tfcard] ▶ navMusicButton clicked");
	_switch_nav(E_NAV_MUSIC);
	return false;
}

static bool onButtonClick_Button1(ZKButton *pButton) {
	LOGD("[tfcard] ▶ Button1 (Video) clicked");
	_switch_nav(E_NAV_VIDEO);
	return false;
}

static bool onButtonClick_navPhotoButton(ZKButton *pButton) {
	LOGD("[tfcard] ▶ navPhotoButton clicked");
	_switch_nav(E_NAV_PHOTO);
	return false;
}
static void obtainListItemData_folderListView(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ folderListView  !!!\n");

	// 边界保护：index 必须在有效范围内
	if (index < 0 || index >= (int)_s_folder_list.size()) {
		LOGD("[tfcard] obtainListItemData_folder: index %d out of range (size=%d), clearing",
			index, (int)_s_folder_list.size());
		// 清空这一行的显示内容
		ZKListView::ZKListSubItem *nameItem = pListItem->findSubItemByID(ID_TFCARD_SubItemName);
		if (nameItem) nameItem->setText("");
		ZKListView::ZKListSubItem *iconItem = pListItem->findSubItemByID(ID_TFCARD_SubItemIcon);
		if (iconItem) iconItem->setBackgroundPic(NULL);
		pListItem->setSelected(false);
		return;
	}

	std::string display_name = _get_folder_display_name(_s_folder_list[index]);

	ZKListView::ZKListSubItem *nameItem = pListItem->findSubItemByID(ID_TFCARD_SubItemName);
	if (nameItem) {
		nameItem->setText(display_name);
	}

	ZKListView::ZKListSubItem *iconItem = pListItem->findSubItemByID(ID_TFCARD_SubItemIcon);
	if (iconItem) {
		iconItem->setBackgroundPic("tfcard/list_folder.png");
	}

	pListItem->setSelected(index == _s_selected_folder_index);

}

static void obtainListItemData_ListView1(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ ListView1  !!!\n");

	if (index < 0 || index >= (int)_s_filtered_file_indices.size()) {
		LOGD("[tfcard] obtainListItemData_file: index %d out of range (size=%d), clearing",
			index, (int)_s_filtered_file_indices.size());
		ZKListView::ZKListSubItem *nameItem = pListItem->findSubItemByID(ID_TFCARD_rightSubItemName);
		if (nameItem) nameItem->setText("");
		ZKListView::ZKListSubItem *iconItem = pListItem->findSubItemByID(ID_TFCARD_rightSubItemIcon);
		if (iconItem) iconItem->setBackgroundPic(NULL);
		return;
	}

	int real_index = _s_filtered_file_indices[index];
	std::string file = _get_file_by_index(real_index);

	ZKListView::ZKListSubItem *nameItem = pListItem->findSubItemByID(ID_TFCARD_rightSubItemName);
	if (nameItem) {
		nameItem->setText(_get_file_name(file));
		nameItem->setLongMode(ZKTextView::E_LONG_MODE_DOTS);
	}

	ZKListView::ZKListSubItem *iconItem = pListItem->findSubItemByID(ID_TFCARD_rightSubItemIcon);
	if (iconItem) {
		switch (_s_current_nav) {
		case E_NAV_MUSIC:
			iconItem->setBackgroundPic("tfcard/list_music.png");
			break;
		case E_NAV_VIDEO:
			iconItem->setBackgroundPic("tfcard/list_video.png");
			break;
		case E_NAV_PHOTO:
			iconItem->setBackgroundPic("tfcard/list_img.png");
			break;
		}
	}
}
