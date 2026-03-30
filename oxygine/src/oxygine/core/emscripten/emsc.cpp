#include "emsc.h"

namespace oxygine
{
    void emscSyncFS(bool read)
    {
#ifdef __EMSCRIPTEN__
        EM_ASM_ARGS(
        {
            FS.mkdir('/data');
            FS.mount(IDBFS, {}, '/data');

            FS.syncfs(true, function(err)
            {
                // handle callback
            });
        }, 0);
#else

#endif
    }
}