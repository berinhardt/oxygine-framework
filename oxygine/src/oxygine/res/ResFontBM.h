#pragma once
#include "../oxygine-include.h"
#include "ResFont.h"
#include "../core/Texture.h"
#include <vector>
#include <list>

namespace oxygine
{
    class ResFontBM: public ResFont
    {
    public:
        static Resource* create(CreateResourceContext& context);
        static Resource* createBM(CreateResourceContext& context);
        static Resource* createSD(CreateResourceContext& context);

        ResFontBM();
        ~ResFontBM();

        /**loads "fnt" font from file, supported XML and text format*/
        void init(const char* fntPath, bool premultipliedAlpha = false);
        void initSD(const char* fntPath, int downsample);
        std::string getPath() const { return _path; }

        void cleanup();

        const Font* getFont(const char* name = 0, int size = 0) const override;
        const Font* getClosestFont(float worldScale, int styleFontSize, float& resScale) const override;

    private:
        struct page
        {
            std::string file;
            spNativeTexture texture;
        };

        void addPage(int tw, int th, const char* head, const char* file);

        void _loadPage(const page& p, LoadResourcesContext*);
        void _load(LoadResourcesContext*) override;
        void _loadFontFromFile(const std::string&, bool sdf, bool bmc, int downsample);
        void _unload() override;
        void _restore(Restorable*, void*);
        void _createFont(CreateResourceContext* context, bool sd, bool bmc, int downsample);
        void _createFontFromTxt(CreateResourceContext* context, char* fontData, const std::string& fontPath, int downsample);
        void _finalize();

        typedef std::vector<page> pages;
        pages _pages;
        Font* _font;
        bool _sdf;

        TextureFormat _format;
        
        bool loaded;
        std::string _file;
        std::string _path;
        bool _premultipliedAlpha;
        float base_scale;
    };
}
