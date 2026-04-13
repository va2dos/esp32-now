#pragma once

#include <stdint.h>
#include <string.h>

namespace services
{

    struct TrackInfo
    {
        uint8_t folder;
        uint8_t file;
        uint32_t duration;
        const char *name;
    };

    enum class TrackParseError : uint8_t
    {
        None = 0,
        InvalidPrefix = 1,
        BadFormat = 2,
        TrackNotFound = 3,
    };

    constexpr TrackInfo TRACKS_REFS[] = {
        {1, 1, 62, "Darkwing Duck"},
        {1, 2, 62, "Ducktales"},
        {1, 3, 159, "Mickey"},
        {1, 4, 89, "Mickey Club"},
        {1, 5, 61, "Mickey Hotdog"},
        {2, 1, 65, "Chip"},
        {2, 2, 61, "Dale"},
        {2, 3, 61, "Gadget"},
        {2, 4, 61, "Jack"},
        {3, 1, 01, "Mario coin"},
    };

    class TrackService
    {
    public:
        TrackParseError parsePlayCommand(const char *cmd, TrackInfo &trackInfo) const;
        void buildPlayCommand(char *out, uint8_t folder, uint8_t file) const;
    };
}