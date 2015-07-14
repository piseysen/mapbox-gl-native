#ifndef MBGL_TEXT_GLYPH_STORE
#define MBGL_TEXT_GLYPH_STORE

#include <mbgl/text/glyph.hpp>
#include <mbgl/util/exclusive.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/work_queue.hpp>

#include <exception>
#include <set>
#include <string>
#include <unordered_map>

namespace mbgl {

class FontStack;
class GlyphPBF;

// The GlyphStore manages the loading and storage of Glyphs
// and creation of FontStack objects. The GlyphStore lives
// on the MapThread but can be queried from any thread.
class GlyphStore {
public:
    class Observer {
    public:
        virtual ~Observer() = default;

        virtual void onGlyphRangeLoaded() = 0;
        virtual void onGlyphRangeLoadingFailed(std::exception_ptr error) = 0;
    };

    GlyphStore();
    ~GlyphStore();

    util::exclusive<FontStack> getFontStack(const std::string& fontStack);

    // Returns true if the set of GlyphRanges are available and parsed or false
    // if they are not. For the missing ranges, a request on the FileSource is
    // made and when the glyph if finally parsed, it gets added to the respective
    // FontStack and a signal is emitted to notify the observers. This method
    // can be called from any thread.
    bool hasGlyphRanges(const std::string& fontStackName, const std::set<GlyphRange>& glyphRanges);

    void setURL(const std::string &url) {
        glyphURL = url;
    }

    void setObserver(Observer* observer);

private:
    void emitGlyphRangeLoaded();
    void emitGlyphRangeLoadingFailed(const std::string& message);

    util::exclusive<FontStack> createFontStack(const std::string &fontStack);
    void requestGlyphRange(const std::string& fontStackName, const GlyphRange& range);

    std::string glyphURL;

    std::unordered_map<std::string, std::map<GlyphRange, std::unique_ptr<GlyphPBF>>> ranges;
    std::mutex rangesMutex;

    std::unordered_map<std::string, std::unique_ptr<FontStack>> stacks;
    std::mutex stacksMutex;

    util::WorkQueue workQueue;

    Observer* observer = nullptr;
};

}

#endif
