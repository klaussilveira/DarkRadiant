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

TEST_F(FilterTest, FiltersLoadedFromGameXML)
{
    bool allReadOnly = true;
    std::set<std::string> filterNames;
    GlobalFilterSystem().forEachFilter([&](const filters::SceneFilter& f) {
        filterNames.insert(f.getName());
        allReadOnly &= f.isReadOnly();
    });

    static const std::set<std::string> EXPECTED_FILTERS{
        "All entities",
        "Brushes",
        "Caulk",
        "Clip Textures",
        "Collision surfaces",
        "Decals",
        "Func_static Entities",
        "Lights",
        "Location Entities",
        "Nodraw Textures",
        "Patches",
        "Paths",
        "Player Start Entity",
        "Shadow Textures",
        "Sky Portals",
        "Trigger Textures",
        "Visportals",
        "Weather Textures",
        "World geometry"
    };
    EXPECT_EQ(filterNames, EXPECTED_FILTERS);
    EXPECT_TRUE(allReadOnly) << "All filters should be read-only";

    // Filters in the game file are read only and inactive by default
    for (const std::string name: EXPECTED_FILTERS) {
        EXPECT_FALSE(GlobalFilterSystem().getFilterState(name));
    }

    // Check some rule sets
    FilterRules lightRules = GlobalFilterSystem().getRuleSet("Lights");
    ASSERT_EQ(lightRules.size(), 2);
    EXPECT_EQ(lightRules[0].type, FilterType::ECLASS);
    EXPECT_EQ(lightRules[0].match, "light");
    EXPECT_FALSE(lightRules[0].show);
    EXPECT_EQ(lightRules[1].type, FilterType::ECLASS);
    EXPECT_EQ(lightRules[1].match, "light_.*");
    EXPECT_FALSE(lightRules[1].show);

    FilterRules brushRules = GlobalFilterSystem().getRuleSet("Brushes");
    ASSERT_EQ(brushRules.size(), 1);
    EXPECT_EQ(brushRules[0].type, FilterType::OBJECT);
    EXPECT_EQ(brushRules[0].match, "brush");

    FilterRules trigRules = GlobalFilterSystem().getRuleSet("Trigger Textures");
    ASSERT_EQ(trigRules.size(), 1);
    EXPECT_EQ(trigRules[0].type, FilterType::TEXTURE);
    EXPECT_EQ(trigRules[0].match, "textures/common/trig(.*)");
}

TEST_F(FilterTest, FilterStates)
{
    int signalCount = 0;
    GlobalFilterSystem().filterConfigChangedSignal().connect([&signalCount]() {
        ++signalCount;
    });

    EXPECT_FALSE(GlobalFilterSystem().getFilterState("Caulk"));
    EXPECT_EQ(signalCount, 0);

    // Signal is currently emitted unconditionally even if nothing actually changed
    GlobalFilterSystem().setFilterState("Caulk", false); // nop
    EXPECT_EQ(signalCount, 1);
    EXPECT_FALSE(GlobalFilterSystem().getFilterState("Caulk"));

    // Actually change some filter states
    GlobalFilterSystem().setFilterState("Caulk", true);
    GlobalFilterSystem().setFilterState("Visportals", true);
    EXPECT_EQ(signalCount, 3);
    EXPECT_TRUE(GlobalFilterSystem().getFilterState("Caulk"));
    EXPECT_TRUE(GlobalFilterSystem().getFilterState("Visportals"));

    GlobalFilterSystem().setFilterState("Visportals", false);
    EXPECT_EQ(signalCount, 4);
    EXPECT_FALSE(GlobalFilterSystem().getFilterState("Visportals"));

    // Pushing state should not change any states
    GlobalFilterSystem().pushState();
    EXPECT_EQ(signalCount, 4);
    EXPECT_TRUE(GlobalFilterSystem().getFilterState("Caulk"));
    EXPECT_FALSE(GlobalFilterSystem().getFilterState("Visportals"));
    EXPECT_FALSE(GlobalFilterSystem().getFilterState("All entities"));

    GlobalFilterSystem().setFilterState("Visportals", true);
    GlobalFilterSystem().setFilterState("All entities", true);
    GlobalFilterSystem().setFilterState("Patches", true);
    EXPECT_EQ(signalCount, 7);

    // Pop state should restore the previous state (and emit the signal since
    // states are changing)
    GlobalFilterSystem().popState();
    EXPECT_EQ(signalCount, 8);
    EXPECT_TRUE(GlobalFilterSystem().getFilterState("Caulk"));
    EXPECT_FALSE(GlobalFilterSystem().getFilterState("Visportals"));
    EXPECT_FALSE(GlobalFilterSystem().getFilterState("All entities"));
    EXPECT_FALSE(GlobalFilterSystem().getFilterState("Patches"));

    // Popping too many times should just do nothing (not crash or throw an exception)
    GlobalFilterSystem().popState();
    GlobalFilterSystem().popState();
    EXPECT_EQ(signalCount, 8);
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
