/**
 * @author    Hugo3132
 * @copyright 2-clause BSD license
 */
#pragma once
#include <ViewBase.h>
#include <map>

namespace views {
/**
 * @brief Class managing which holds static references to all views which can be
 * accessed over the ViewId without including any views. This ensures that no
 * include loops are necessary.
 *
 * To activate a view use
 * views::ViewStore::activateView(views::ViewStore::HardwareTestView);
 */
class ViewStore {
public:
  /**
   * @brief IDs of the different views
   */
  enum ViewId {
    EmergencyEnterKeyView,
    EmergencyEnterMenuView,
    EmergencyMenu,
    EmlalockUnlockKeyMenu,
    HardwareTestView,
    HygieneOpeningMenu,
    LockedView,
    PreferencesMenu,
    SelectDisplayTimeLeft,
    SelectDisplayTimePassed,
    SetTimerView,
    UnlockedMainMenu,
    UnlockSafeView,
    WifiConnectingView,
  };

protected:
  /**
   * @brief Map linking the IDs
   */
  std::map<ViewId, lcd::ViewBase*> views;

protected:
  /**
   * @brief protected, use the static functions instead
   */
  ViewStore() {};

protected:
  /**
   * @brief Get the Singleton object if the ViewStore
   */
  static ViewStore& getSingleton() {
    static ViewStore instance;
    return instance;
  }

public:
  /**
   * @brief Returns the pointer to the view registered by the given id
   *
   * @param id @ref ViewId of the view which should be returned
   * @return pointer to the requested view or nullptr if the view was not
   * registered (yet)
   */
  static lcd::ViewBase* getView(const ViewId& id) {
    auto it = getSingleton().views.find(id);
    if (it == getSingleton().views.end()) {
      return nullptr;
    }
    return it->second;
  }

public:
  /**
   * @brief registers a new view
   *
   * @param id @ref ViewId of the view which should be registered
   * @param view pointer to the view which should be accessible using the
   * @ref ViewId
   */
  static void addView(const ViewId& id, lcd::ViewBase& view) {
    getSingleton().views[id] = &view;
  }

public:
  /**
   * @brief activates the view described by the id
   *
   * @param id @ref ViewId of the view which should be activated
   * @return true if the view was known
   */
  static bool activateView(const ViewId& id) {
    auto it = getSingleton().views.find(id);
    if (it == getSingleton().views.end()) {
      lcd::ViewBase::activateView(nullptr);
      return false;
    }

    lcd::ViewBase::activateView(it->second);
    return true;
  }
};
} // namespace views