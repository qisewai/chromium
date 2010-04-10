// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_GTK_LOCATION_BAR_VIEW_GTK_H_
#define CHROME_BROWSER_GTK_LOCATION_BAR_VIEW_GTK_H_

#include <gtk/gtk.h>

#include <map>
#include <string>

#include "app/gtk_signal.h"
#include "base/basictypes.h"
#include "base/scoped_ptr.h"
#include "base/scoped_vector.h"
#include "chrome/browser/autocomplete/autocomplete_edit.h"
#include "chrome/browser/autocomplete/autocomplete_edit_view_gtk.h"
#include "chrome/browser/extensions/extension_context_menu_model.h"
#include "chrome/browser/extensions/image_loading_tracker.h"
#include "chrome/browser/gtk/info_bubble_gtk.h"
#include "chrome/browser/gtk/menu_gtk.h"
#include "chrome/browser/location_bar.h"
#include "chrome/common/content_settings_types.h"
#include "chrome/common/notification_observer.h"
#include "chrome/common/notification_registrar.h"
#include "chrome/common/owned_widget_gtk.h"
#include "chrome/common/page_transition_types.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "webkit/glue/window_open_disposition.h"

class AutocompleteEditViewGtk;
class BubblePositioner;
class Browser;
class CommandUpdater;
class ContentSettingImageModel;
class ContentSettingBubbleGtk;
class ExtensionAction;
class GtkThemeProvider;
class Profile;
class SkBitmap;
class TabContents;
class ToolbarModel;

class LocationBarViewGtk : public AutocompleteEditController,
                           public LocationBar,
                           public LocationBarTesting,
                           public NotificationObserver {
 public:
  LocationBarViewGtk(const BubblePositioner* bubble_positioner,
                     Browser* browser_);
  virtual ~LocationBarViewGtk();

  void Init(bool popup_window_mode);

  void SetProfile(Profile* profile);

  // Returns the widget the caller should host.  You must call Init() first.
  GtkWidget* widget() { return hbox_.get(); }

  // Returns the current TabContents.
  TabContents* GetTabContents() const;

  // Sets |preview_enabled| for the PageActionViewGtk associated with this
  // |page_action|. If |preview_enabled| is true, the view will display the
  // page action's icon even though it has not been activated by the extension.
  // This is used by the ExtensionInstalledBubbleGtk to preview what the icon
  // will look like for the user upon installation of the extension.
  void SetPreviewEnabledPageAction(ExtensionAction *page_action,
                                   bool preview_enabled);

  // Retrieves the GtkWidget which is associated with PageActionView
  // corresponding to |page_action|.
  GtkWidget* GetPageActionWidget(ExtensionAction* page_action);

  // Updates the location bar.  We also reset the bar's permanent text and
  // security style, and, if |tab_for_state_restoring| is non-NULL, also
  // restore saved state that the tab holds.
  void Update(const TabContents* tab_for_state_restoring);

  // Show the bookmark bubble.
  void ShowStarBubble(const GURL& url, bool newly_boomkarked);

  // Set the starred state of the bookmark star.
  void SetStarred(bool starred);

  // Implement the AutocompleteEditController interface.
  virtual void OnAutocompleteAccept(const GURL& url,
      WindowOpenDisposition disposition,
      PageTransition::Type transition,
      const GURL& alternate_nav_url);
  virtual void OnChanged();
  virtual void OnKillFocus();
  virtual void OnSetFocus();
  virtual void OnInputInProgress(bool in_progress);
  virtual SkBitmap GetFavIcon() const;
  virtual std::wstring GetTitle() const;

  // Implement the LocationBar interface.
  virtual void ShowFirstRunBubble(bool use_OEM_bubble);
  virtual std::wstring GetInputString() const;
  virtual WindowOpenDisposition GetWindowOpenDisposition() const;
  virtual PageTransition::Type GetPageTransition() const;
  virtual void AcceptInput();
  virtual void AcceptInputWithDisposition(WindowOpenDisposition);
  virtual void FocusLocation();
  virtual void FocusSearch();
  virtual void UpdateContentSettingsIcons();
  virtual void UpdatePageActions();
  virtual void InvalidatePageActions();
  virtual void SaveStateToContents(TabContents* contents);
  virtual void Revert();
  virtual const AutocompleteEditView* location_entry() const {
    return location_entry_.get();
  }
  virtual AutocompleteEditView* location_entry() {
    return location_entry_.get();
  }
  virtual LocationBarTesting* GetLocationBarForTesting() { return this; }

  // Implement the LocationBarTesting interface.
  virtual int PageActionCount() { return page_action_views_.size(); }
  virtual int PageActionVisibleCount();
  virtual ExtensionAction* GetPageAction(size_t index);
  virtual ExtensionAction* GetVisiblePageAction(size_t index);
  virtual void TestPageActionPressed(size_t index);

  // Implement the NotificationObserver interface.
  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details);

  // Edit background color.
  static const GdkColor kBackgroundColor;

 private:
  class ContentSettingImageViewGtk : public InfoBubbleGtkDelegate {
   public:
    ContentSettingImageViewGtk(ContentSettingsType content_type,
                               const LocationBarViewGtk* parent,
                               Profile* profile);
    virtual ~ContentSettingImageViewGtk();

    GtkWidget* widget() { return event_box_.get(); }

    void set_profile(Profile* profile) { profile_ = profile; }

    bool IsVisible() { return GTK_WIDGET_VISIBLE(widget()); }
    void UpdateFromTabContents(const TabContents* tab_contents);

   private:
    CHROMEGTK_CALLBACK_1(ContentSettingImageViewGtk, gboolean, OnButtonPressed,
                         GdkEvent*);

    // InfoBubbleDelegate overrides:
    virtual void InfoBubbleClosing(InfoBubbleGtk* info_bubble,
                                   bool closed_by_escape);

    scoped_ptr<ContentSettingImageModel> content_setting_image_model_;

    // The widgets for this content settings view.
    OwnedWidgetGtk event_box_;
    OwnedWidgetGtk image_;

    // The owning LocationBarViewGtk.
    const LocationBarViewGtk* parent_;

    // The currently active profile.
    Profile* profile_;

    // The currently shown info bubble if any.
    ContentSettingBubbleGtk* info_bubble_;

    DISALLOW_COPY_AND_ASSIGN(ContentSettingImageViewGtk);
  };

  class PageActionViewGtk : public ImageLoadingTracker::Observer,
                            public ExtensionContextMenuModel::PopupDelegate {
   public:
    PageActionViewGtk(
        LocationBarViewGtk* owner, Profile* profile,
        ExtensionAction* page_action);
    virtual ~PageActionViewGtk();

    GtkWidget* widget() { return event_box_.get(); }

    ExtensionAction* page_action() { return page_action_; }

    void set_preview_enabled(bool preview_enabled) {
      preview_enabled_ = preview_enabled;
    }

    bool IsVisible() { return GTK_WIDGET_VISIBLE(widget()); }

    // Called to notify the PageAction that it should determine whether to be
    // visible or hidden. |contents| is the TabContents that is active, |url|
    // is the current page URL.
    void UpdateVisibility(TabContents* contents, GURL url);

    // A callback from ImageLoadingTracker for when the image has loaded.
    virtual void OnImageLoaded(
        SkBitmap* image, ExtensionResource resource, int index);

    // Simulate left mouse click on the page action button.
    void TestActivatePageAction();

    // Overridden from ExtensionContextMenuModel::PopupDelegate:
    virtual void InspectPopup(ExtensionAction* action);

   private:
    // Show the popup for this page action. If |devtools| is true, show it
    // with a debugger window attached. Returns true if a popup was shown.
    bool ShowPopup(bool devtools);

    CHROMEGTK_CALLBACK_1(PageActionViewGtk, gboolean, OnButtonPressed,
                         GdkEvent*);
    CHROMEGTK_CALLBACK_1(PageActionViewGtk, gboolean, OnExposeEvent,
                         GdkEventExpose*);

    // The location bar view that owns us.
    LocationBarViewGtk* owner_;

    // The current profile (not owned by us).
    Profile* profile_;

    // The PageAction that this view represents. The PageAction is not owned by
    // us, it resides in the extension of this particular profile.
    ExtensionAction* page_action_;

    // A cache of all the different icon paths associated with this page action.
    typedef std::map<std::string, GdkPixbuf*> PixbufMap;
    PixbufMap pixbufs_;

    // A cache of the last dynamically generated bitmap and the pixbuf that
    // corresponds to it. We keep track of both so we can free old pixbufs as
    // their icons are replaced.
    SkBitmap last_icon_skbitmap_;
    GdkPixbuf* last_icon_pixbuf_;

    // The object that is waiting for the image loading to complete
    // asynchronously.
    ImageLoadingTracker tracker_;

    // The widgets for this page action.
    OwnedWidgetGtk event_box_;
    OwnedWidgetGtk image_;

    // The tab id we are currently showing the icon for.
    int current_tab_id_;

    // The URL we are currently showing the icon for.
    GURL current_url_;

    // This is used for post-install visual feedback. The page_action icon
    // is briefly shown even if it hasn't been enabled by its extension.
    bool preview_enabled_;

    // The context menu view and model for this extension action.
    scoped_ptr<MenuGtk> context_menu_;
    scoped_refptr<ExtensionContextMenuModel> context_menu_model_;

    DISALLOW_COPY_AND_ASSIGN(PageActionViewGtk);
  };
  friend class PageActionViewGtk;

  // Creates, initializes, and packs the location icon + related widgets.
  void BuildLocationIcon();

  GtkWidget* location_icon() { return location_icon_alignment_; }

  CHROMEGTK_CALLBACK_1(LocationBarViewGtk, gboolean, HandleExpose,
                       GdkEventExpose*);
  CHROMEGTK_CALLBACK_1(LocationBarViewGtk, gboolean, OnIconReleased,
                       GdkEventButton*);
  CHROMEGTK_CALLBACK_4(LocationBarViewGtk, void, OnIconDragData,
                       GdkDragContext*, GtkSelectionData*, guint, guint);
  CHROMEGTK_CALLBACK_1(LocationBarViewGtk, void, OnEntryBoxSizeAllocate,
                       GtkAllocation*);
  CHROMEGTK_CALLBACK_1(LocationBarViewGtk, gboolean, OnStarButtonPress,
                       GdkEventButton*);

  // Updates the location_icon_box_'s icon.
  void UpdateIcon();

  // Sets the text that should be displayed in the info label and its associated
  // tooltip text.  Call with an empty string if the info label should be
  // hidden.
  void SetInfoText();

  // Set the keyword text for the Search BLAH: keyword box.
  void SetKeywordLabel(const std::wstring& keyword);

  // Set the keyword text for the "Press tab to search BLAH" hint box.
  void SetKeywordHintLabel(const std::wstring& keyword);

  void ShowFirstRunBubbleInternal(bool use_OEM_bubble);

  // Show or hide |tab_to_search_box_|, |tab_to_search_hint_| and
  // |type_to_search_hint_| according to the value of |show_selected_keyword_|,
  // |show_keyword_hint_|, |show_search_hint_| and the available horizontal
  // space in the location bar.
  void AdjustChildrenVisibility();

  // Build the star icon.
  void CreateStarButton();

  // Update the star icon after it is toggled or the theme changes.
  void UpdateStarIcon();

  // Returns true if we should only show the URL and none of the extras like
  // the star button or page actions.
  bool ShouldOnlyShowLocation();

  // The outermost widget we want to be hosted.
  OwnedWidgetGtk hbox_;

  // Star button.
  OwnedWidgetGtk star_;
  GtkWidget* star_image_;
  bool starred_;

  // SSL state.
  GtkWidget* security_icon_event_box_;
  GtkWidget* ev_secure_icon_image_;
  GtkWidget* secure_icon_image_;
  GtkWidget* security_warning_icon_image_;
  GtkWidget* security_error_icon_image_;
  // An icon to the left of the address bar.
  GtkWidget* location_icon_alignment_;
  GtkWidget* location_icon_image_;
  // TODO(pkasting): Split this label off and move the rest of the items to the
  // left of the address bar.
  GtkWidget* security_info_label_;

  // Content setting icons.
  OwnedWidgetGtk content_setting_hbox_;
  ScopedVector<ContentSettingImageViewGtk> content_setting_views_;

  // Extension page action icons.
  OwnedWidgetGtk page_action_hbox_;
  ScopedVector<PageActionViewGtk> page_action_views_;

  // Area on the left shown when in tab to search mode.
  GtkWidget* tab_to_search_box_;
  GtkWidget* tab_to_search_full_label_;
  GtkWidget* tab_to_search_partial_label_;

  // Hint to user that they can tab-to-search by hitting tab.
  GtkWidget* tab_to_search_hint_;
  GtkWidget* tab_to_search_hint_leading_label_;
  GtkWidget* tab_to_search_hint_icon_;
  GtkWidget* tab_to_search_hint_trailing_label_;

  // Hint to user that the inputted text is not a keyword or url.
  GtkWidget* type_to_search_hint_;

  scoped_ptr<AutocompleteEditViewGtk> location_entry_;

  Profile* profile_;
  CommandUpdater* command_updater_;
  ToolbarModel* toolbar_model_;
  Browser* browser_;

  // We need to hold on to this just to it pass to the edit.
  const BubblePositioner* bubble_positioner_;

  // When we get an OnAutocompleteAccept notification from the autocomplete
  // edit, we save the input string so we can give it back to the browser on
  // the LocationBar interface via GetInputString().
  std::wstring location_input_;

  // The user's desired disposition for how their input should be opened.
  WindowOpenDisposition disposition_;

  // The transition type to use for the navigation.
  PageTransition::Type transition_;

  // Used schedule a task for the first run info bubble.
  ScopedRunnableMethodFactory<LocationBarViewGtk> first_run_bubble_;

  // When true, the location bar view is read only and also is has a slightly
  // different presentation (font size / color). This is used for popups.
  bool popup_window_mode_;

  // Provides colors and rendering mode.
  GtkThemeProvider* theme_provider_;

  NotificationRegistrar registrar_;

  // Width of the hbox that holds |tab_to_search_box_|, |location_entry_| and
  // |tab_to_search_hint_|.
  int entry_box_width_;

  // Indicate if |tab_to_search_box_| should be shown.
  bool show_selected_keyword_;

  // Indicate if |tab_to_search_hint_| should be shown.
  bool show_keyword_hint_;

  // Indicate if |type_to_search_hint_| should be shown.
  bool show_search_hint_;

  DISALLOW_COPY_AND_ASSIGN(LocationBarViewGtk);
};

#endif  // CHROME_BROWSER_GTK_LOCATION_BAR_VIEW_GTK_H_
