#include <imgui.h>
#include <nfd.h>
#include <string>
#include "common.h"
#include "core.h"

namespace ic::ui {
namespace dialog {

    static nfdu8filteritem_t filters[]
        = { { "Logic Circuit Simulation File", "ic" } };

    LCS_ERROR open_file(void)
    {
        nfdu8char_t* out = nullptr;
        nfdopendialogu8args_t args {};
        args.filterList  = filters;
        args.filterCount = 1;

        nfdresult_t result = NFD_OpenDialogU8_With(&out, &args);
        if (result == NFD_ERROR) {
            L_ERROR("%s", NFD_GetError());
            return Error::NFD;
        } else if (result == NFD_OKAY) {
            std::vector<uint8_t> data;
            Error err = tabs::open(out);
            NFD_FreePathU8(out);
            return err;
        }
        return Error::OK;
    }

    LCS_ERROR save_file_as()
    {
        nfdu8char_t* out = nullptr;
        nfdsavedialogu8args_t args {};
        args.filterList    = filters;
        args.filterCount   = 1;
        nfdresult_t result = NFD_SaveDialogU8_With(&out, &args);
        if (result == NFD_ERROR) {
            L_ERROR("%s", NFD_GetError());
            return Error::NFD;
        }
        if (result == NFD_OKAY) {
            std::string path = out;
            if (path.rfind(".ic") == std::string::npos) {
                L_DEBUG("File is not an .ic file. Adding file extension.");
                path += ".ic";
            }
            if (Error err = tabs::save_as(path); err) {
                return err;
            }
            NFD_FreePathU8(out);
        }
        return Error::OK;
    }
} // namespace dialog

} // namespace ic::ui
