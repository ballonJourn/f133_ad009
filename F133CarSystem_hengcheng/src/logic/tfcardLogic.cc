#pragma once
#include "uart/ProtocolSender.h"

/*
 * tfcardLogic.cc
 * TF Card 媒体浏览器 —— 左侧文件夹列表 + 右侧文件列表 + 底部类型切换
 *
 * 设计逻辑：
 * 1. 进入界面 → 默认 Music 模式 → 显示 loading → 扫描 SD 卡
 * 2. 左侧列表：从扫描结果中提取不同的文件夹路径，按文件夹分组
 * 3. 点击左侧文件夹 → 右侧列表过滤显示该文件夹下的文件
 * 4. 点击右侧文件 → 跳转到对应播放器 (musicActivity / videoActivity / PhotoAlbumActivity)
 * 5. 底部导航栏切换 Music/Video/Photo → 重新分组，刷新两侧列表
 */

#include "media/media_context.h"
#include "media/music_player.h"
#include "system/setting.h"
#include "link/context.h"
#include <set>
#include <algorithm>

// ==================== 类型定义 ====================

// 当前浏览的媒体类型
typedef enum {
	E_NAV_MUSIC = 0,
	E_NAV_VIDEO,
	E_NAV_PHOTO,
} nav_type_e;

// ==================== 状态变量 ====================

static nav_type_e _s_current_nav = E_NAV_MUSIC;           // 当前导航类型
static int _s_selected_folder_index = -1;                  // 左侧选中的文件夹索引
static storage_type_e _s_storage = E_STORAGE_TYPE_SD;      // 当前存储类型

// 文件夹列表：存储去重后的文件夹路径
static std::vector<std::string> _s_folder_list;

// 右侧文件列表：当前文件夹下过滤后的文件索引(指向 media 全局列表的 index)
static std::vector<int> _s_filtered_file_indices;

#define SCAN_CHECK_TIMER   1
#define LOADING_TIMEOUT    2

// ==================== 辅助函数 ====================

/**
 * 从完整文件路径提取文件夹路径
 * 例如: "/mnt/extsd/music/rock/song.mp3" → "/mnt/extsd/music/rock"
 */
static std::string _get_folder_path(const std::string &file_path) {
	size_t pos = file_path.rfind('/');
	if (pos == std::string::npos || pos == 0) {
		return "/";
	}
	return file_path.substr(0, pos);
}

/**
 * 从文件夹路径提取显示名称（最后一级目录名）
 * 例如: "/mnt/extsd/music/rock" → "rock"
 */
static std::string _get_folder_display_name(const std::string &folder_path) {
	size_t pos = folder_path.rfind('/');
	if (pos == std::string::npos || pos >= folder_path.size() - 1) {
		return folder_path;
	}
	return folder_path.substr(pos + 1);
}

/**
 * 从完整文件路径提取文件名
 */
static std::string _get_file_name(const std::string &file_path) {
	size_t pos = file_path.rfind('/');
	if (pos == std::string::npos) {
		return file_path;
	}
	return file_path.substr(pos + 1);
}

/**
 * 获取当前导航类型对应的文件总数
 */
static int _get_total_file_count() {
	switch (_s_current_nav) {
	case E_NAV_MUSIC: return media::get_audio_list_size(_s_storage);
	case E_NAV_VIDEO: return media::get_video_list_size(_s_storage);
	case E_NAV_PHOTO: return media::get_image_list_size(_s_storage);
	}
	return 0;
}

/**
 * 获取当前导航类型的第 index 个文件路径
 */
static std::string _get_file_by_index(int index) {
	switch (_s_current_nav) {
	case E_NAV_MUSIC: return media::get_audio_file(_s_storage, index);
	case E_NAV_VIDEO: return media::get_video_file(_s_storage, index);
	case E_NAV_PHOTO: return media::get_image_file(_s_storage, index);
	}
	return "";
}

/**
 * 重建文件夹列表 —— 从当前类型的所有文件中提取去重的文件夹路径
 */
static void _rebuild_folder_list() {
	_s_folder_list.clear();
	std::set<std::string> folder_set;

	int total = _get_total_file_count();
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

	// 按字母排序
	std::sort(_s_folder_list.begin(), _s_folder_list.end());

	LOGD("[tfcard] rebuilt folder list: %d folders for nav=%d",
		(int)_s_folder_list.size(), _s_current_nav);
}

/**
 * 根据选中的文件夹，过滤右侧文件列表
 */
static void _rebuild_file_list() {
	_s_filtered_file_indices.clear();

	if (_s_selected_folder_index < 0 ||
		_s_selected_folder_index >= (int)_s_folder_list.size()) {
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

	LOGD("[tfcard] filtered files: %d files in folder [%s]",
		(int)_s_filtered_file_indices.size(), target_folder.c_str());
}

/**
 * 刷新全部UI
 */
static void _refresh_all() {
	_rebuild_folder_list();

	// 自动选中第一个文件夹
	if (!_s_folder_list.empty()) {
		_s_selected_folder_index = 0;
	} else {
		_s_selected_folder_index = -1;
	}

	_rebuild_file_list();

	if (mfolderListViewPtr) {
		mfolderListViewPtr->refreshListView();
	}
	if (mListView1Ptr) {
		mListView1Ptr->refreshListView();
	}
}

/**
 * 更新底部导航栏选中态
 */
static void _update_nav_selection() {
	if (mnavMusicButtonPtr) {
		mnavMusicButtonPtr->setSelected(_s_current_nav == E_NAV_MUSIC);
	}
	if (mButton1Ptr) {
		mButton1Ptr->setSelected(_s_current_nav == E_NAV_VIDEO);
	}
	if (mnavPhotoButtonPtr) {
		mnavPhotoButtonPtr->setSelected(_s_current_nav == E_NAV_PHOTO);
	}
}

/**
 * 切换导航类型
 */
static void _switch_nav(nav_type_e nav) {
	if (_s_current_nav == nav) {
		return;
	}
	_s_current_nav = nav;
	_update_nav_selection();

	// 显示loading
	if (mloadingPopupWindowPtr) {
		mloadingPopupWindowPtr->showWnd();
	}
	if (mloadingPointerPtr) {
		mloadingPointerPtr->setTargetAngle(360);
	}

	// 延迟刷新，让 loading 先显示
	mActivityPtr->registerUserTimer(SCAN_CHECK_TIMER, 100);
}

/**
 * 显示加载弹窗
 */
static void _show_loading() {
	if (mloadingPopupWindowPtr) {
		mloadingPopupWindowPtr->showWnd();
	}
}

/**
 * 隐藏加载弹窗
 */
static void _hide_loading() {
	if (mloadingPopupWindowPtr) {
		mloadingPopupWindowPtr->hideWnd();
	}
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
	LOGD("[tfcard] onUI_init");
	_s_current_nav = E_NAV_MUSIC;
	_s_selected_folder_index = -1;
	_s_storage = E_STORAGE_TYPE_SD;
	_s_folder_list.clear();
	_s_filtered_file_indices.clear();

	media::add_scan_cb(_media_scan_cb);
}

static void onUI_intent(const Intent *intentPtr) {
	if (intentPtr != NULL) {
		// 可以通过 intent 指定初始类型
		std::string nav = intentPtr->getExtra("nav_type");
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
	LOGD("[tfcard] onUI_show");
	_update_nav_selection();

	// 检查扫描状态
	if (media::is_scanning()) {
		_show_loading();
	} else {
		// 扫描已完成，直接刷新
		_refresh_all();

		int total = _get_total_file_count();
		if (total == 0) {
			// 没有文件 → 可能SD卡未插入或无媒体文件
			_show_loading();
			// 设置超时自动关闭loading
			mActivityPtr->registerUserTimer(LOADING_TIMEOUT, 2000);
		}
	}
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
		_refresh_all();
		_hide_loading();
		return false;
	case LOADING_TIMEOUT:
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



// ==================== 右侧文件列表回调 ====================



// ==================== 底部导航按钮 ====================

static bool onButtonClick_navMusicButton(ZKButton *pButton) {
	LOGD("[tfcard] ButtonClick navMusicButton");
	_switch_nav(E_NAV_MUSIC);
	return false;
}

static bool onButtonClick_Button1(ZKButton *pButton) {
	LOGD("[tfcard] ButtonClick Button1 (Video)");
	_switch_nav(E_NAV_VIDEO);
	return false;
}

static bool onButtonClick_navPhotoButton(ZKButton *pButton) {
	LOGD("[tfcard] ButtonClick navPhotoButton");
	_switch_nav(E_NAV_PHOTO);
	return false;
}
static void obtainListItemData_folderListView(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {


	if (index < 0 || index >= (int)_s_folder_list.size()) {
		return;
	}

	// 设置文件夹名称
	ZKListView::ZKListSubItem *nameItem =
		pListItem->findSubItemByID(ID_TFCARD_SubItemName);
	if (nameItem) {
		std::string display = _get_folder_display_name(_s_folder_list[index]);
		nameItem->setText(display);
	}

	// 设置文件夹图标
	ZKListView::ZKListSubItem *iconItem =
		pListItem->findSubItemByID(ID_TFCARD_SubItemIcon);
	if (iconItem) {
		iconItem->setBackgroundPic("tfcard/list_folder.png");
	}

	// 选中态
	pListItem->setSelected(index == _s_selected_folder_index);
}

static void onListItemClick_folderListView(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ folderListView  !!!\n");

	LOGD("[tfcard] folder click: index=%d", index);

	if (index < 0 || index >= (int)_s_folder_list.size()) {
		return;
	}

	_s_selected_folder_index = index;

	// 重建右侧文件列表
	_rebuild_file_list();

	// 刷新两侧列表
	mfolderListViewPtr->refreshListView();
	if (mListView1Ptr) {
		mListView1Ptr->refreshListView();
	}

}

static void obtainListItemData_ListView1(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ ListView1  !!!\n");


	if (index < 0 || index >= (int)_s_filtered_file_indices.size()) {
		return;
	}

	int real_index = _s_filtered_file_indices[index];
	std::string file = _get_file_by_index(real_index);

	// 文件名
	ZKListView::ZKListSubItem *nameItem =
		pListItem->findSubItemByID(ID_TFCARD_rightSubItemName);
	if (nameItem) {
		nameItem->setText(_get_file_name(file));
		nameItem->setLongMode(ZKTextView::E_LONG_MODE_DOTS);
	}

	// 类型图标
	ZKListView::ZKListSubItem *iconItem =
		pListItem->findSubItemByID(ID_TFCARD_rightSubItemIcon);
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

static void onListItemClick_ListView1(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ ListView1  !!!\n");


	LOGD("[tfcard] file click: index=%d, subItem id=%d", index, id);

	if (index < 0 || index >= (int)_s_filtered_file_indices.size()) {
		return;
	}

	int real_index = _s_filtered_file_indices[index];

	// 如果处于互联连接状态，阻止打开本地媒体
	if (lk::is_connected()) {
		LOGD("[tfcard] link connected, ignore file click");
		return;
	}

	switch (_s_current_nav) {
	case E_NAV_MUSIC: {
		// musicActivity.onUI_intent 是空的，不接受外部参数
		// 正确做法：先通过 music_play 设置播放，再打开界面
		sys::setting::set_music_play_dev(E_AUDIO_TYPE_MUSIC);
		media::music_play(_s_storage, real_index);
		EASYUICONTEXT->openActivity("musicActivity");
		break;
	}
	case E_NAV_VIDEO: {
		// videoActivity.onUI_intent 也是空的
		// 视频播放的流程：videoActivity 自己管理 _s_play_index，
		// 从 tfcard 进入时，先直接打开 videoActivity，
		// 用户在 videoActivity 的列表里点击播放
		// TODO: 后续可扩展 videoActivity.onUI_intent 接受参数
		EASYUICONTEXT->openActivity("videoActivity");
		break;
	}
	case E_NAV_PHOTO: {
		// PhotoAlbumActivity 同样不接受外部 intent
		EASYUICONTEXT->openActivity("PhotoAlbumActivity");
		break;
	}
	}

}
static int getListItemCount_folderListView(const ZKListView *pListView) {
    //LOGD("getListItemCount_folderListView !\n");
    return 5;
}

static int getListItemCount_ListView1(const ZKListView *pListView) {
    //LOGD("getListItemCount_ListView1 !\n");
    return 5;
}
