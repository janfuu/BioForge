#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace BioForge::Web
{
    struct CommitResult
    {
        bool        ok{};
        std::string note;   // human-readable outcome, shown in the review row
    };

    // Write one character bio into SkyrimNet's content library through the
    // dashboard's own HTTP API (the loopback web server). This is the only
    // supported write path in Beta 25: SkyrimNet no longer polls prompt
    // files, so a file dropped on disk by anything other than its
    // ContentStore is invisible until some unrelated event rescans the
    // library. Routing through the API gets the store's own rescan and the
    // template-cache invalidation that makes the bio go live in seconds -
    // the same mechanism the dashboard's Characters page uses.
    //
    // Mirrors that flow exactly: POST /characters?api=create (name, actor
    // UUID, content), and when the character file already exists - HTTP 409,
    // which is how a re-commit presents - PUT /characters?api=update with
    // the file's content path.
    //
    // Pure HTTP + file reads, no RE:: calls: safe from any thread, and the
    // caller runs it on a worker so the UI thread never blocks on a socket.
    CommitResult CreateOrUpdateBio(std::string_view a_displayName,
                                   std::uint64_t a_actorUUID, std::string_view a_stem,
                                   std::string_view a_content);

    // Whether the loopback server is configured and enabled at all - read
    // from <SkyrimNet>/config/WebServer.yaml. A commit against a disabled
    // server fails with a note pointing here.
    bool ServerEnabled();

    // Everything this actor's own records were written to say, conditions NOT
    // evaluated - the lines a quest only unlocks later included. available_
    // dialogue() in the prompt sees only what the engine allows right now, so
    // a quest character early in their story reads as whatever their idle
    // lines make them (an innkeeper whose whole questline is still locked).
    // Quest and scene dialogue only; generic and combat lines are left out.
    //
    // Returns the raw JSON of GET /game-data?api=actor-dialogue (the
    // dashboard's Actor Dialogue page), or "" on any failure. The prompt
    // filters it; nothing here parses it. Pure HTTP, but the endpoint reads
    // game data: call it from a WORKER, never the main thread.
    std::string FetchAuthoredDialogue(std::uint32_t a_refFormID);
}
