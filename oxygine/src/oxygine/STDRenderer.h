#pragma once
#include "oxygine-include.h"
#include "core/Renderer.h"
#include "MaterialX.h"

namespace oxygine
{
    class SDFMaterial : public STDMaterialX
    {
    public:
        MATX(SDFMaterial);

        void init() override;

        Vector4 outlineColor = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
        Vector4 outlineParams = Vector4(0.0f, 0.0f, 0.0f, 0.0f);


        void rehash(size_t& hash) const override;

        static bool cmp(const SDFMaterial& a, const SDFMaterial& b);

        void xapply();
        void render(const Color& c, const RectF& src, const RectF& dest) override;
    };

    class STDRenderer : public IElementRenderer
    {
    public:

        static STDRenderer* current;
        static STDRenderer* instance;
        /**Initializes internal classes. Called automatically from oxygine::init();*/
        static void initialize();
        /**Clears internal data*/
        static void release();
        /**for lost context*/
        static void reset();
        /**is Renderer was restored and ready to be used*/
        static bool isReady();
        /**restore after lost context*/
        static void restore();

        /**returns activated renderer with STDRenderer::begin*/
        static STDRenderer* getCurrent();

        /**White 4x4 Texture*/
        static spNativeTexture white;
        static spNativeTexture invisible;

        static UberShaderProgram uberShader;
        static std::vector<unsigned char> uberShaderBody;
        static std::vector<unsigned short> indices16;
        static size_t maxVertices;



        STDRenderer(IVideoDriver* driver = 0);
        virtual ~STDRenderer();

        const Matrix&   getViewProjection() const;
        IVideoDriver*   getDriver();
        unsigned int    getShaderFlags() const;
        const VertexDeclaration* getVertexDeclaration() const { return _vdecl; }

        void setShaderFlags(int);
        void setViewProj(const Matrix& viewProj);
        void setVertexDeclaration(const VertexDeclaration* decl);
        void setUberShaderProgram(UberShaderProgram* pr);
        /**Sets blend mode. Default value is blend_premultiplied_alpha*/
        void setBlendMode(blend_mode blend);
        /**Sets texture. If texture is null White texture would be used.*/
        void setTexture(const spNativeTexture& base, const spNativeTexture& alpha, bool basePremultiplied = true);
        void setTexture(const spNativeTexture& base, bool basePremultiplied = true);
        /**Sets World transformation.*/
        void setTransform(const Transform& world);
        void draw(const Color&, const RectF& srcRect, const RectF& destRect);
        void setTextureNew(int sampler, const spNativeTexture& t);

        /**Draws existing batch immediately.*/
        void drawBatch();


        void applySDF(float contrast, float offset, const Color& outlineColor, float outlineOffset);
        void endSDF();

        void applySimpleMode(bool basePremultiplied);
        /**used in pair with applySimpleMode/applySDF, fast, don't have excess checks*/
        void draw(const spNativeTexture& texture, unsigned int color, const RectF& src, const RectF& dest) override;

        /*
        void draw(MaterialX* mat, const Color& color, const RectF& src, const RectF& dest) override;
        void draw(MaterialX* mat, const AffineTransform& transform, const Color& color, const RectF& src, const RectF& dest);
        void draw(MaterialX* mat, vertexPCT2 vert[4]);
        */

        /**Begins rendering into RenderTexture or into primary framebuffer if rt is null*/
        void begin();
        void begin(spNativeTexture renderTarget, const Rect* viewport = 0);
        /**Completes started rendering and restores previous Frame Buffer.*/
        void end();
        /**initializes View + Projection matrices where TopLeft is (0,0) and RightBottom is (width, height). use flipU = true for render to texture*/
        void initCoordinateSystem(int width, int height, bool flipU = false);
        void resetSettings();
        void process();
        void process(int);
        void flush();

        virtual void addVertices(const void* data, unsigned int size);


        //debug utils
#ifdef OXYGINE_DEBUG_T2P
        static void showTexel2PixelErrors(bool show);
#endif

        void swapVerticesData(std::vector<unsigned char>& data);
        void swapVerticesData(STDRenderer& r);

        OXYGINE_DEPRECATED
        void setViewProjTransform(const Matrix& viewProj);

    protected:

        struct batch
        {
            spMaterialX mat;
            std::vector<vertexPCT2> vertices;
            RectF bbox;
        };

        typedef std::vector<batch> batches;
        batches _batches;

        enum { MAX_TEXTURES = 4 };
        spNativeTexture _textures[MAX_TEXTURES];

        batch& draw(spMaterialX mat);

        int _baseShaderFlags;
        Transform _transform;

        STDRenderer* _previous;
        void setShader(ShaderProgram* prog);

        void xdrawBatch();


        void xaddVertices(const void* data, unsigned int size);
        void checkDrawBatch();

        std::vector<unsigned char> _vertices;

        const VertexDeclaration* _vdecl;

        IVideoDriver* _driver;
        ShaderProgram* _program;
        Matrix _vp;

        virtual void preDrawBatch();
        virtual void xbegin();
        virtual void xresetSettings();
        //virtual
        spNativeTexture _base;
        spNativeTexture _alpha;

        blend_mode _blend;

        UberShaderProgram* _uberShader;
        unsigned int _shaderFlags;

        bool _drawing;

        spNativeTexture _prevRT;
    };


    typedef void(*render_texture_hook)(const spNativeTexture& nt);
    void set_render_texture_hook(render_texture_hook);
    render_texture_hook get_render_texture_hook();
}