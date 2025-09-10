#include <cstdint>
#include "core.h"

namespace lcs::tabs {

class Tab : public Scene {
public:
    Tab(bool _is_saved, std::filesystem::path _path, Scene _scene)
        : Scene { std::move(_scene) }
        , is_saved { _is_saved }
        , path { _path } { };

    bool is_saved;
    std::filesystem::path path;
};
static std::vector<Tab> TABS;
static size_t selected     = SIZE_MAX;
static size_t old_selected = SIZE_MAX;

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
    Tab inode { true, path, Scene {} };
    if (Error err = inode.read_from(data); err) {
        return err;
    }
    TABS.push_back(std::move(inode));
    old_selected = selected;
    selected     = TABS.size() - 1;
    L_INFO("Scene \"%s\" is opened.", TABS[selected].name().data());
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
    lcs_assert(idx < TABS.size());
    return TABS[idx].is_saved;
}

LCS_ERROR save(size_t idx)
{
    if (idx == SIZE_MAX) {
        idx = selected;
    }
    lcs_assert(idx < TABS.size());
    Tab& inode = TABS[idx];
    if (inode.is_saved) {
        return OK;
    }
    std::vector<uint8_t> scene_bin;
    Error err = inode.write_to(scene_bin);
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
    lcs_assert(idx < TABS.size());
    Tab& inode = TABS[idx];
    if (inode.path == new_path) {
        return OK;
    }
    std::filesystem::path oldpath = inode.path;
    inode.path                    = new_path;
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
    return &TABS[selected];
}

size_t create(const std::string& name, const std::string& author,
    const std::string& description, int version)
{
    TABS.emplace_back(false, "", Scene { name, author, description, version });
    old_selected = selected;
    selected     = TABS.size() - 1;
    L_INFO("Scene \"%s\"(%zu) is opened.", name.c_str(), selected);
    return selected;
}

void for_each(std::function<bool(std::string_view name,
        const std::filesystem::path& path, bool is_saved, bool is_active)>
        run)
{
    size_t updated_scene = selected;
    for (size_t i = 0; i < TABS.size(); i++) {
        if (run(std::string_view { TABS[i].name().data() }, TABS[i].path,
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

} // namespace lcs::tabs
