#pragma once
#include "uart/ProtocolSender.h"

/*
 * phonelinkLogic.cc — 统一投屏选择页（参照 v553 LinkLogic.cc）
 *
 * v553 的做法：
 * - 点击 tab → 如果和当前模式不同 → 弹窗"切换至 XXX 模式吗？"
 *            → 如果和当前模式相同 → 什么都不做
 * - 用户点确认 → 执行 _confirm_link_mode（切换网络模式 + BT + 重启 lylink）
 *                + _set_tab_selected（切换 tab 选中态 + 帮助图窗口）
 * - 用户点取消 → 关闭弹窗
 */

#include "net/context.h"
#include "net/ap.h"
#include "bt/context.h"
#include "link/context.h"
#include "system/setting.h"
#include "common.h"
#include "mode_observer.h"

#define LINK_RESTART_TIMER    1

// ==================== 状态 ====================

static link_mode_e _s_target_link_mode;  // 弹窗待确认的目标模式

// ==================== Tab 选中态 + Window 切换 ====================

static void _set_tip_status(link_mode_e mode) {
	LOGD("[phonelink] _set_tip_status: %s", sys::setting::get_link_mode_str(mode));

	// Tab 选中态
	if (mcarplayTabButtonPtr)  mcarplayTabButtonPtr->setSelected(mode == E_LINK_MODE_CARPLAY);
	if (mandroidTabButtonPtr)  mandroidTabButtonPtr->setSelected(mode == E_LINK_MODE_ANDROIDAUTO);
	if (mairplayTabButtonPtr)  mairplayTabButtonPtr->setSelected(mode == E_LINK_MODE_AIRPLAY);
	if (mmiracastTabButtonPtr) mmiracastTabButtonPtr->setSelected(mode == E_LINK_MODE_MIRACAST);

	// Window 显示/隐藏
	if (mcpWindowPtr) mcpWindowPtr->setVisible(mode == E_LINK_MODE_CARPLAY);
	if (maaWindowPtr) maaWindowPtr->setVisible(mode == E_LINK_MODE_ANDROIDAUTO);
	if (mapWindowPtr) mapWindowPtr->setVisible(mode == E_LINK_MODE_AIRPLAY);
	if (mmcWindowPtr) mmcWindowPtr->setVisible(mode == E_LINK_MODE_MIRACAST);
}

// ==================== 弹窗文字 + 显示 ====================

static void _set_tip_text(link_mode_e mode) {
	LOGD("[phonelink] _set_tip_text: showing confirm for %s",
		sys::setting::get_link_mode_str(mode));

	if (mtipInfoTextPtr) {
		std::string msg = std::string("Switch to \"")
			+ sys::setting::get_link_mode_str(mode)
			+ "\" mode?";
		mtipInfoTextPtr->setText(msg);
		LOGD("[phonelink]   text: %s", msg.c_str());
	}

	// 显示弹窗
	if (mtipMaskWindowPtr) {
		mtipMaskWindowPtr->showWnd();
		LOGD("[phonelink]   tipMaskWindow shown");
	} else {
		LOGD("[phonelink]   ERROR: mtipMaskWindowPtr is NULL");
	}
}

// ==================== 网络模式切换 + BT + lylink 重启 ====================

static void _confirm_link_mode(link_mode_e mode) {
	link_mode_e last_mode = sys::setting::get_link_mode();
	LOGD("[phonelink] _confirm_link_mode: %s → %s",
		sys::setting::get_link_mode_str(last_mode),
		sys::setting::get_link_mode_str(mode));

	sys::setting::set_link_mode(mode);

	switch (mode) {
	case E_LINK_MODE_CARPLAY:
	case E_LINK_MODE_ANDROIDAUTO:
		// CarPlay / AA 需要蓝牙开 + AP 模式
		if (!bt::is_on()) {
			LOGD("[phonelink]   turning ON BT");
			bt::power_on();
		}
		if (net::get_mode() == E_NET_MODE_AP) {
			if ((last_mode == E_LINK_MODE_AIRPLAY) || (mode == E_LINK_MODE_AIRPLAY)) {
				LOGD("[phonelink]   restarting hostapd");
				net::ap::restart_hostapd();
			}
		} else {
			LOGD("[phonelink]   changing net to AP");
			net::change_mode(E_NET_MODE_AP);
		}
		break;

	case E_LINK_MODE_AIRPLAY:
		// AirPlay 需要蓝牙关 + AP 模式
		if (bt::is_on()) {
			LOGD("[phonelink]   turning OFF BT");
			bt::power_off();
		}
		if (net::get_mode() == E_NET_MODE_AP) {
			if (last_mode != E_LINK_MODE_AIRPLAY) {
				LOGD("[phonelink]   restarting hostapd");
				net::ap::restart_hostapd();
			}
		} else {
			LOGD("[phonelink]   changing net to AP");
			net::change_mode(E_NET_MODE_AP);
		}
		break;

	case E_LINK_MODE_MIRACAST:
	case E_LINK_MODE_LYLINK:
		// Miracast / AICast 需要蓝牙关 + P2P 模式
		if (bt::is_on()) {
			LOGD("[phonelink]   turning OFF BT");
			bt::power_off();
		}
		LOGD("[phonelink]   changing net to P2P");
		net::change_mode(E_NET_MODE_P2P);
		break;

	default:
		break;
	}

	LOGD("[phonelink]   scheduling lylink restart");
	mActivityPtr->registerUserTimer(LINK_RESTART_TIMER, 10);
}

// ==================== 核心：tab 点击 → 判断是否需要弹窗 ====================

/**
 * 参照 v553 _select_link_mode：
 * - 当前模式 ≠ 目标模式 → 弹窗确认
 * - 当前模式 == 目标模式 → 不操作
 */
static void _select_link_mode(link_mode_e mode) {
	LOGD("[phonelink] _select_link_mode: target=%s, current=%s",
		sys::setting::get_link_mode_str(mode),
		sys::setting::get_link_mode_str(sys::setting::get_link_mode()));

	if (sys::setting::get_link_mode() != mode) {
		// 模式不同 → 弹窗确认
		_s_target_link_mode = mode;
		_set_tip_text(mode);
	} else {
		LOGD("[phonelink]   same mode, no action");
	}
}

// ==================== 帮助步骤文字 (参照 linkhelpLogic.cc _update_layout) ====================

static void _update_step_text() {
	std::string bt_name = sys::setting::get_bt_name();
	std::string dev_name = sys::setting::get_dev_name();

	// CarPlay 步骤文字
	std::string cpStep = LTOV("cp_step1") + bt_name + "\n\n"
					   + LTOV("cp_step2");

	// Android Auto 步骤文字
	std::string aaStep = LTOV("aa_step1") + "\n\n"
					   + LTOV("aa_step2") + bt_name;

	// AirPlay 步骤文字
	std::string apStep = LTOV("ap_step1") + dev_name + "\n\n"
					   + LTOV("ap_step2") + std::string("12345678") + "\n\n"
					   + LTOV("ap_step3") + dev_name;

	// Miracast 步骤文字
	std::string mcStep = LTOV("mc_step1") + "\n\n"
					   + LTOV("mc_step2") + "\n\n"
					   + LTOV("mc_step3") + dev_name;

	if (mcpStepTextViewPtr) mcpStepTextViewPtr->setText(cpStep);
	if (maaStepTextViewPtr) maaStepTextViewPtr->setText(aaStep);
	if (mapStepTextViewPtr) mapStepTextViewPtr->setText(apStep);
	if (mmcStepTextViewPtr) mmcStepTextViewPtr->setText(mcStep);

	// cpTipTextView 也设置帮助标题（与 linkhelp 保持一致）
	if (mcpTipTextViewPtr) mcpTipTextViewPtr->setText(LTOV("cp_help"));
}

// ==================== 生命周期 ====================

static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000},
};

static void onUI_init() {
	LOGD("[phonelink] ========== onUI_init ==========");
	_s_target_link_mode = sys::setting::get_link_mode();
	LOGD("[phonelink]   saved link_mode: %s (%d)",
		sys::setting::get_link_mode_str(_s_target_link_mode), _s_target_link_mode);

	// 设置各模式的帮助步骤文字
	_update_step_text();
}

static void onUI_intent(const Intent *intentPtr) {
	LOGD("[phonelink] onUI_intent");
	if (intentPtr != NULL) {
		std::string mode_str = intentPtr->getExtra("link_mode");
		if (!mode_str.empty()) {
			link_mode_e mode = (link_mode_e)atoi(mode_str.c_str());
			LOGD("[phonelink]   intent link_mode=%d (%s)", mode,
				sys::setting::get_link_mode_str(mode));
			// 直接显示 intent 指定的模式（不弹窗）
			_set_tip_status(mode);
		}
	}
}

static void onUI_show() {
	LOGD("[phonelink] ========== onUI_show ==========");

	link_mode_e current = sys::setting::get_link_mode();
	LOGD("[phonelink]   current saved mode: %s, connected: %d",
		sys::setting::get_link_mode_str(current), lk::is_connected());
	LOGD("[phonelink]   current net_mode: %d", net::get_mode());

	mode::set_switch_mode(E_SWITCH_MODE_NULL);

	// 显示当前已保存模式对应的 tab 和帮助图
	_set_tip_status(current);

	// 确保弹窗初始隐藏
	if (mtipMaskWindowPtr) {
		mtipMaskWindowPtr->hideWnd();
	}

	// 检查网络模式是否与当前 link_mode 匹配，不匹配则修正
	// 参照 linkhelpLogic.cc _select_link_mode 的检查逻辑
	net_mode_e expected_net = E_NET_MODE_AP;
	switch (current) {
	case E_LINK_MODE_CARPLAY:
	case E_LINK_MODE_ANDROIDAUTO:
	case E_LINK_MODE_AIRPLAY:
		expected_net = E_NET_MODE_AP;
		break;
	case E_LINK_MODE_CARLIFE:
		expected_net = E_NET_MODE_WIFI;
		break;
	case E_LINK_MODE_MIRACAST:
	case E_LINK_MODE_LYLINK:
		expected_net = E_NET_MODE_P2P;
		break;
	default:
		expected_net = E_NET_MODE_AP;
		break;
	}

	if (net::get_mode() != expected_net) {
		LOGD("[phonelink]   net mode mismatch: current=%d, expected=%d → fixing",
			net::get_mode(), expected_net);
		_confirm_link_mode(current);
	} else {
		LOGD("[phonelink]   net mode OK (%d), lylink should be running", expected_net);
	}
}

static void onUI_hide() {
	LOGD("[phonelink] onUI_hide");
}

static void onUI_quit() {
	LOGD("[phonelink] onUI_quit");
}

static void onProtocolDataUpdate(const SProtocolData &data) {
}

static bool onUI_Timer(int id) {
	switch (id) {
	case LINK_RESTART_TIMER:
		LOGD("[phonelink] LINK_RESTART_TIMER → lk::restart_lylink()");
		lk::restart_lylink();
		return false;
	default:
		break;
	}
	return true;
}

static bool onphonelinkActivityTouchEvent(const MotionEvent &ev) {
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

// ==================== Tab 按钮回调 ====================

static bool onButtonClick_carplayTabButton(ZKButton *pButton) {
	LOGD("[phonelink] ▶ tab: CarPlay");
	_select_link_mode(E_LINK_MODE_CARPLAY);
	return false;
}

static bool onButtonClick_androidTabButton(ZKButton *pButton) {
	LOGD("[phonelink] ▶ tab: Android Auto");
	_select_link_mode(E_LINK_MODE_ANDROIDAUTO);
	return false;
}

static bool onButtonClick_airplayTabButton(ZKButton *pButton) {
	LOGD("[phonelink] ▶ tab: AirPlay");
	_select_link_mode(E_LINK_MODE_AIRPLAY);
	return false;
}

static bool onButtonClick_miracastTabButton(ZKButton *pButton) {
	LOGD("[phonelink] ▶ tab: Miracast");
	_select_link_mode(E_LINK_MODE_MIRACAST);
	return false;
}

// ==================== 弹窗按钮回调 ====================

static bool onButtonClick_tipConfirmButton(ZKButton *pButton) {
	LOGD("[phonelink] ▶ CONFIRM → switching to %s",
		sys::setting::get_link_mode_str(_s_target_link_mode));

	// 1. 关闭弹窗
	if (mtipMaskWindowPtr) {
		mtipMaskWindowPtr->hideWnd();
	}

	// 2. 执行网络模式切换 + BT + lylink 重启
	_confirm_link_mode(_s_target_link_mode);

	// 3. 切换 tab 选中态 + 帮助图窗口
	_set_tip_status(_s_target_link_mode);

	return false;
}

static bool onButtonClick_tipCancelButton(ZKButton *pButton) {
	LOGD("[phonelink] ▶ CANCEL");
	if (mtipMaskWindowPtr) {
		mtipMaskWindowPtr->hideWnd();
	}
	return false;
}
