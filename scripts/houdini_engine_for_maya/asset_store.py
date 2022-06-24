from __future__ import print_function
import json
import os
import re
import sys

import maya.cmds as cmds
import maya.mel as mel

current_user = None

asset_size = 100
child_entries_margin = 5
child_assets_column = 3

asset_store_window = "houdiniEngineAssetStoreWindow"
change_user_menu_item = "houdiniEngineAssetStoreChangeUserMenuItem"
asset_entries_scroll_layout = "houdiniEngineAssetStoreEntriesScrollLayout"

def get_store_path():
    if "HOUDINI_ASSET_STORE_PATH" in os.environ:
        return os.environ["HOUDINI_ASSET_STORE_PATH"]

    houdini_version = cmds.houdiniEngine(houdiniVersion = True).split(".")

    # Get Houdini prefs directory
    houdini_prefs_path = None
    if sys.platform.startswith("linux"):
        houdini_prefs_path = os.path.expanduser(
                "~/houdini{0}.{1}".format(
                    houdini_version[0], houdini_version[1]
                    )
                )
    elif sys.platform.startswith("win32"):
        houdini_prefs_path = os.path.expanduser(
                "~/houdini{0}.{1}".format(
                    houdini_version[0], houdini_version[1]
                    )
                )
    elif sys.platform.startswith("darwin"):
        houdini_prefs_path = os.path.expanduser(
                "~/Library/Preferences/houdini/{0}.{1}".format(
                    houdini_version[0], houdini_version[1]
                    )
                )
    else:
        raise Exception("Cannot determine asset store path. Unknown OS.")

    asset_store_path = os.path.join(houdini_prefs_path, "asset_store")

    return asset_store_path

def get_store_user_path():
    user = get_store_current_user()

    users_root = get_users()

    user_dir = None

    if users_root and user in users_root["users"]:
        user_dir = users_root["users"][user]

    if not user_dir:
        user_dir = "default"

    return os.path.join(get_store_path(), user_dir)

def get_store_users_path():
    return os.path.join(get_store_path(), "users.json")

def get_store_installed_assets_path():
    return os.path.join(get_store_user_path(), "installed_assets.json")

def get_store_otls_path():
    return os.path.join(get_store_user_path(), "otls")

def get_store_icons_path():
    return os.path.join(get_store_user_path(), "icons")

def get_store_licenses_path():
    return os.path.join(get_store_user_path(), "licenses")

def get_store_current_user():
    global current_user

    if not current_user:
        users_root = get_users()
        if users_root and "default_user" in users_root:
            current_user = users_root["default_user"]

    if not current_user:
        print("Warning: Cannot determine default user for asset store.")

    return current_user

def get_users():
    users_json = get_store_users_path()

    if not os.path.exists(users_json):
        return None

    users_root = None
    with open(users_json, "r") as f:
        users_root = json.load(f)

    return users_root

def get_installed_assets():
    installed_assets_json = get_store_installed_assets_path()

    if not os.path.exists(installed_assets_json):
        return None

    installed_assets_root = None
    with open(installed_assets_json, "r") as f:
        installed_assets_root = json.load(f)

    return installed_assets_root

def get_asset_license(otl_file):
    license_json = os.path.join(get_store_licenses_path(),
            re.sub("\\.otl$|\\.hda$|\\.otllc$|\\.hdalc$|\\.otlnc$|\\.hdanc$", ".json", otl_file))

    license_root = None
    with open(license_json, "r") as f:
        license_root = json.load(f)

    return license_root

def load_asset(otl_file, asset):
    cmds.houdiniAsset(loadAsset=[otl_file, asset])

def create_asset_entry(asset):
    form_layout = cmds.formLayout(width = asset_size, height = asset_size)

    if "otl_file" in asset:
        license = get_asset_license(asset["otl_file"])

        otl_file = os.path.join(get_store_otls_path(), asset["otl_file"])

        m = re.match("([^:]*)::(.*)", asset["node_type_name"])
        asset_name = "{0}::{1}/{2}".format(
                m.group(1),
                license["category_name"],
                m.group(2),
                )

        cmds.symbolButton(
                annotation = asset["descriptive_name"],
                image = os.path.join(get_store_icons_path(), asset["icon"]),
                width = asset_size, height = asset_size,
                command = lambda *args: load_asset(otl_file, asset_name)
                )
    elif "update_available" in asset and asset["update_available"]:
        cmds.symbolButton(
                annotation = asset["descriptive_name"],
                image = os.path.join(get_store_icons_path(), asset["icon"]),
                width = asset_size, height = asset_size,
                )
        cmds.text(
                label = "Update available. Use Houdini to update asset.",
                width = asset_size, height = asset_size,
                wordWrap = True,
                )

    text = cmds.text(
            label = asset["descriptive_name"],
            backgroundColor = [0,0,0],
            align = "right",
            )
    cmds.formLayout(
            form_layout,
            edit = True,
            width = asset_size, height = asset_size,
            attachForm = [[text, "left", 0], [text, "right", 0], [text, "bottom", 0]],
            )

    cmds.setParent(upLevel = True)

def compare_asset_entry(x, y):
    if x["type"] == "folder" and y["type"] == "folder":
        if x["name"] < y["name"]:
            return -1
        else:
            return 1
    elif x["type"] == "asset" and y["type"] == "asset":
        return -1
    elif x["type"] == "asset" and y["type"] == "folder":
        return -1
    elif x["type"] == "folder" and y["type"] == "asset":
        return 1

def create_asset_entries(entries):
    in_assets_layout = False

    cmds.columnLayout(adjustableColumn = True)

    for entry in sorted(entries, cmp = compare_asset_entry):
        if entry["type"] == "folder":
            if in_assets_layout:
                in_assets_layout = False
                cmds.setParent(upLevel=True)

            cmds.frameLayout(
                    collapsable = True,
                    label = entry["name"],
                    marginWidth = child_entries_margin
                    )

            create_asset_entries(entry["entries"])

            cmds.setParent(upLevel = True)
        elif entry["type"] == "asset":
            if not in_assets_layout:
                in_assets_layout = True
                cmds.gridLayout(
                        numberOfColumns = child_assets_column,
                        cellWidthHeight = [asset_size, asset_size]
                        )

            create_asset_entry(entry)

    if in_assets_layout:
        in_assets_layout = False
        cmds.setParent(upLevel = True)

    cmds.setParent(upLevel = True)

def change_user(user):
    current_user = user

    refresh_asset_entries()

def change_user_post_menu_command(*args):
    cmds.menu(
            change_user_menu_item,
            edit = True,
            deleteAllItems = True,
            )

    cmds.setParent(change_user_menu_item, menu = True)

    users_root = get_users()

    if not users_root:
        return

    current_user = get_store_current_user()

    for user in users_root["users"]:
        cmds.menuItem(
                label = user,
                command = lambda user=user: change_user(user),
                radioButton = user == current_user
                )

def refresh_asset_entries(*args):
    if not cmds.window(asset_store_window, exists = True):
        return

    cmds.setParent(asset_store_window)

    # Delete the existing layout
    if cmds.scrollLayout(asset_entries_scroll_layout, exists = True):
        cmds.deleteUI(asset_entries_scroll_layout)

    installed_assets = get_installed_assets()

    cmds.scrollLayout(asset_entries_scroll_layout, childResizable = True)

    if installed_assets \
            and installed_assets["organization"]["entries"]:
        create_asset_entries(installed_assets["organization"]["entries"])
    else:
        cmds.text(
                label = "There are no Orbolt assets installed for this user.<br />"
                "Please visit the <a href=\"http://www.orbolt.com/maya\">Orbolt Store</a> for assets.",
                wordWrap = True,
                hyperlink = True,
                )


    cmds.setParent(upLevel = True)

def close_asset_store_window(*args):
    cmds.deleteUI(asset_store_window)

def show_asset_store_window():
    if cmds.window(asset_store_window, exists = True):
        cmds.showWindow(asset_store_window)
        return

    cmds.window(
            asset_store_window,
            title = "Orbolt Asset Browser",
            menuBar = True,
            )

    cmds.menu(label = "File", tearOff = True)

    cmds.menuItem(
            change_user_menu_item,
            label = "Change User",
            subMenu = True,
            postMenuCommand = change_user_post_menu_command,
            )
    cmds.setParent(menu = True, upLevel = True)

    cmds.menuItem(divider = True)
    cmds.menuItem(label = "Refresh Asset List", command = refresh_asset_entries)
    cmds.menuItem(divider = True)
    cmds.menuItem(label = "Close", command = close_asset_store_window)

    cmds.setParent(menu = True, upLevel = True)

    refresh_asset_entries()

    cmds.showWindow(asset_store_window)

