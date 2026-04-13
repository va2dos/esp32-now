#include "services/track_service.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

namespace services
{

    TrackParseError TrackService::parsePlayCommand(const char *cmd, TrackInfo &trackInfo) const
    {
        // Must start with "PLAY-"
        if (strncmp(cmd, "PLAY-", 5) != 0)
        {
            return TrackParseError::InvalidPrefix;
        }

        unsigned int folder = 0;
        unsigned int file = 0;

        // Parse "PLAY-%02u-%03u"
        if (sscanf(cmd + 5, "%2u-%3u", &folder, &file) != 2)
        {
            return TrackParseError::BadFormat;
        }

        // Get TrackInfo from TRACKS_REFS
        for (const auto &track : TRACKS_REFS)
        {
            if (track.folder == folder && track.file == file)
            {
                trackInfo = track;
                return TrackParseError::None;
            }
        }

        return TrackParseError::TrackNotFound;
    }

    void TrackService::buildPlayCommand(char *out, uint8_t folder, uint8_t file) const
    {
        sprintf(out, "PLAY-%02u-%03u", folder, file);
    }

}