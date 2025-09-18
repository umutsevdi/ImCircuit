#include <cstdint>
#include "core.h"

namespace ic::tabs {

static std::vector<Tab> TABS;
static size_t selected     = SIZE_MAX;
static size_t old_selected = SIZE_MAX;

size_t create(const std::string& name, const std::string& author,
    const std::string& description, int version)
{
    TABS.emplace_back(Scene { name, author, description, version });
    old_selected = selected;
    selected     = TABS.size() - 1;
    L_INFO("Scene \"%s\"(%zu) is opened.", name.c_str(), selected);
    return selected;
}

LCS_ERROR open(const std::filesystem::path& path)
{
    for (size_t i = 0; i < TABS.size(); i++) {
        if (TABS[i].path == path) {
            if (i != selected) {
                old_selected = selected;
                selected     = i;
            }
            return OK;
        }
    }
    std::vector<uint8_t> data;
    if (!fs::read(path, data)) {
        return ERROR(Error::NOT_FOUND);
    }
    Tab inode { Scene {}, true, path };
    if (Error err = inode.scene.read_from(data); err) {
        return err;
    }
    TABS.push_back(std::move(inode));
    old_selected = selected;
    selected     = TABS.size() - 1;
    L_INFO("Scene \"%s\" is opened.", TABS[selected].scene.name().data());
    return OK;
}

LCS_ERROR close(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = selected;
    }
    if (TABS.empty() || idx >= TABS.size()) {
        return ERROR(Error::INVALID_TAB);
    }
    if (idx != selected) {
        L_INFO("Tab %zu is closed.", idx);
    } else {
        L_INFO("Active tab(%zu) is closed.", selected);
    }
    TABS.erase(TABS.begin() + idx);
    if (TABS.empty()) {
        selected = SIZE_MAX;
    } else {
        selected--;
        if (!TABS.empty()) {
            old_selected = SIZE_MAX;
        }
    }
    return OK;
}

bool is_saved(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = selected;
    }
    ic_assert(idx < TABS.size());
    return TABS[idx].is_saved;
}

LCS_ERROR save(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = selected;
    }
    ic_assert(idx < TABS.size());
    Tab& inode = TABS[idx];
    if (inode.is_saved) {
        return OK;
    }
    std::vector<uint8_t> scene_bin;
    Error err = inode.scene.write_to(scene_bin);
    if (err) {
        return err;
    }
    if (!fs::write(inode.path, scene_bin)) {
        return ERROR(Error::NO_SAVE_PATH_DEFINED);
    }
    inode.is_saved = true;
    if (idx != selected) {
        L_INFO("Tab %zu is saved.", idx);
    } else {
        L_INFO("Active tab(%zu) is saved.", selected);
    }
    return OK;
}

LCS_ERROR save_as(const std::filesystem::path& new_path, size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = selected;
    }
    ic_assert(idx < TABS.size());
    Tab& inode = TABS[idx];
    if (inode.path == new_path) {
        return OK;
    }
    std::string pathstr = new_path.string();
    if (pathstr.rfind(".imcircuit") == std::string::npos) {
        L_DEBUG("File is not an .imcircuit file. Adding file extension.");
        pathstr += ".imcircuit";
    }
    std::filesystem::path oldpath = inode.path;
    inode.path                    = pathstr;
    inode.is_saved                = false;
    Error err                     = save(idx);
    if (err) {
        inode.path     = oldpath;
        inode.is_saved = true;
        return err;
    }
    return OK;
}

Ref<Scene> active(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = selected;
    }
    if (idx >= TABS.size()) {
        return nullptr;
    }
    if (idx != selected) {
        L_INFO("Switching to tab %zu.", idx);
        old_selected = selected;
        selected     = idx;
    }
    return &TABS[selected].scene;
}

//??
std::vector<Tab>::iterator iterator(void) { return TABS.begin(); }

void for_each(std::function<bool(std::string_view name,
        const std::filesystem::path& path, bool is_saved, bool is_active)>
        run)
{
    size_t updated_scene = selected;
    for (size_t i = 0; i < TABS.size(); i++) {
        if (run(std::string_view { TABS[i].scene.name().data() }, TABS[i].path,
                TABS[i].is_saved, i == selected)) {
            updated_scene = i;
        };
    }
    if (selected != updated_scene) {
        old_selected = selected;
    }
    selected = updated_scene;
}

bool is_changed(void)
{
    if (selected >= TABS.size()) {
        return false;
    }
    if (old_selected != selected) {
        old_selected = selected;
        L_DEBUG("Tab(%d) is updated", selected);
        return true;
    }
    return false;
}

} // namespace ic::tabs
