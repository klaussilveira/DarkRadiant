#include "RadiantTest.h"

#include "ifilter.h"
#include "scene/Node.h"
#include "imap.h"
#include "scene/filters/SceneFilter.h"
#include "scenelib.h"

namespace test
{

class DummyNode: public scene::Node
{
public:
    DummyNode()
    {}

    std::size_t onFiltersChangedInvocationCount = 0;

    void onFiltersChanged() override
    {
        ++onFiltersChangedInvocationCount;
    }

    AABB localAABB() const override
    {
        return {};
    }

    void onPreRender(const VolumeTest& volume) override {}
    void renderHighlights(IRenderableCollector& collector, const VolumeTest& volume) override {}
    std::size_t getHighlightFlags() override { return Highlight::NoHighlight; }
    Type getNodeType() const override { return scene::INode::Type::Unknown; }
};

using FilterTest = RadiantTest;

TEST_F(FilterTest, ConstructSceneFilter)
{
    // Normal filter
    {
        filters::SceneFilter filter("StuffToHide", false);
        EXPECT_EQ(filter.getName(), "StuffToHide");
        EXPECT_EQ(filter.getEventName(), "FilterStuffToHide");
        EXPECT_FALSE(filter.isReadOnly());
        EXPECT_EQ(filter.getRuleSet().size(), 0);
    }

    // Read-only filter
    {
        filters::SceneFilter roFilter("ROFilter", true);
        EXPECT_TRUE(roFilter.isReadOnly());
    }
}

TEST_F(FilterTest, RenameSceneFilter)
{
    filters::SceneFilter f("OriginalName", false);
    EXPECT_EQ(f.getName(), "OriginalName");

    f.setName("AdjustedName");
    EXPECT_EQ(f.getName(), "AdjustedName");
    EXPECT_EQ(f.getEventName(), "FilterAdjustedName");
}

TEST_F(FilterTest, FilterRules)
{
    // Texture-based filtering
    filters::SceneFilter filter("HideStuff", false);
    filter.addRule(FilterType::TEXTURE, "textures/darkmod/badtex", false);

    EXPECT_TRUE(filter.isVisible(FilterType::TEXTURE, "textures/darkmod/good"));
    EXPECT_FALSE(filter.isVisible(FilterType::TEXTURE, "textures/darkmod/badtex"));
    EXPECT_TRUE(filter.isVisible(FilterType::ECLASS, "textures/darkmod/badtex"));
    EXPECT_TRUE(filter.isVisible(FilterType::TEXTURE, "textures/darkmod/badtex1"));

    // Entity class filtering
    scene::INodePtr worldNode = GlobalMapModule().findOrInsertWorldspawn();
    Entity* worldEnt = worldNode->tryGetEntity();
    ASSERT_TRUE(worldEnt);

    filter.addRule(FilterType::ECLASS, "func_static", false);
    EXPECT_TRUE(filter.isEntityVisible(FilterType::ECLASS, *worldEnt));

    filter.addRule(FilterType::ECLASS, "worldspawn", false);
    EXPECT_FALSE(filter.isEntityVisible(FilterType::ECLASS, *worldEnt));
}

TEST_F(FilterTest, OnFiltersChangedInvoked)
{
    auto worldspawn = GlobalMapModule().findOrInsertWorldspawn();
    auto testNode = std::make_shared<DummyNode>();
    scene::addNodeToContainer(testNode, worldspawn);

    EXPECT_EQ(testNode->onFiltersChangedInvocationCount, 0) << "Count should be 0 at first";

    // Set the filter
    GlobalFilterSystem().setFilterState("Caulk", true);

    EXPECT_EQ(testNode->onFiltersChangedInvocationCount, 1) << "Node should have been notified";
}

}
