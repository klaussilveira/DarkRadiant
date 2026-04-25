#include "gtest/gtest.h"

#include "DarkmodTxt.h"

namespace test
{

TEST(DarkmodTxt, EmptyContentReturnsDefaults)
{
    auto info = map::DarkmodTxt::CreateFromString("");

    EXPECT_TRUE(info->getTitle().empty());
    EXPECT_TRUE(info->getAuthor().empty());
    EXPECT_TRUE(info->getDescription().empty());
    EXPECT_TRUE(info->getVersion().empty());
    EXPECT_TRUE(info->getReqTdmVersion().empty());
}

TEST(DarkmodTxt, WhitespaceOnlyContentReturnsDefaults)
{
    auto info = map::DarkmodTxt::CreateFromString("   \n\t\r\n  ");

    EXPECT_TRUE(info->getTitle().empty());
    EXPECT_TRUE(info->getAuthor().empty());
    EXPECT_TRUE(info->getDescription().empty());
}

TEST(DarkmodTxt, ParsesWikiOrder)
{
    std::string input =
        "Title: My Map\n"
        "Description: A great map\n"
        "Author: Me";

    auto info = map::DarkmodTxt::CreateFromString(input);

    EXPECT_EQ(info->getTitle(), "My Map");
    EXPECT_EQ(info->getDescription(), "A great map");
    EXPECT_EQ(info->getAuthor(), "Me");
}

TEST(DarkmodTxt, ParsesOutOfOrderFieldsWithoutThrowing)
{
    std::string input =
        "Title: My Map\n"
        "Author: Me\n"
        "Description: A great map";

    auto info = map::DarkmodTxt::CreateFromString(input);

    EXPECT_EQ(info->getTitle(), "My Map");
    EXPECT_EQ(info->getAuthor(), "Me");
    EXPECT_EQ(info->getDescription(), "A great map");
}

TEST(DarkmodTxt, ParsesAllFields)
{
    std::string input =
        "Title: My Campaign\n"
        "Description: Multi-mission campaign\n"
        "Author: Me\n"
        "Version: 1.2\n"
        "Required TDM Version: v2.10";

    auto info = map::DarkmodTxt::CreateFromString(input);

    EXPECT_EQ(info->getTitle(), "My Campaign");
    EXPECT_EQ(info->getDescription(), "Multi-mission campaign");
    EXPECT_EQ(info->getAuthor(), "Me");
    EXPECT_EQ(info->getVersion(), "1.2");
    EXPECT_EQ(info->getReqTdmVersion(), "2.10");
}

TEST(DarkmodTxt, ReqTdmVersionWithoutVPrefix)
{
    std::string input =
        "Title: My Map\n"
        "Required TDM Version: 2.11";

    auto info = map::DarkmodTxt::CreateFromString(input);

    EXPECT_EQ(info->getReqTdmVersion(), "2.11");
}

TEST(DarkmodTxt, ParsesMissionTitlesForCampaign)
{
    std::string input =
        "Title: My Campaign\n"
        "Mission 1 Title: Opening\n"
        "Mission 2 Title: Climax\n"
        "Description: A great campaign\n"
        "Author: Me";

    auto info = map::DarkmodTxt::CreateFromString(input);

    const auto& titles = info->getMissionTitles();
    ASSERT_EQ(titles.size(), 3u);
    EXPECT_EQ(titles[0], "My Campaign");
    EXPECT_EQ(titles[1], "Opening");
    EXPECT_EQ(titles[2], "Climax");
    EXPECT_EQ(info->getDescription(), "A great campaign");
    EXPECT_EQ(info->getAuthor(), "Me");
}

TEST(DarkmodTxt, ToStringWritesWikiOrder)
{
    map::DarkmodTxt info;
    info.setTitle("My Map");
    info.setAuthor("Me");
    info.setDescription("A great map");
    info.setVersion("1.0");
    info.setReqTdmVersion("2.10");

    std::string out = info.toString();

    auto titlePos = out.find("Title:");
    auto descPos = out.find("Description:");
    auto authorPos = out.find("Author:");
    auto versionPos = out.find("\nVersion:");
    auto reqVersionPos = out.find("Required TDM Version:");

    ASSERT_NE(titlePos, std::string::npos);
    ASSERT_NE(descPos, std::string::npos);
    ASSERT_NE(authorPos, std::string::npos);
    ASSERT_NE(versionPos, std::string::npos);
    ASSERT_NE(reqVersionPos, std::string::npos);

    EXPECT_LT(titlePos, descPos);
    EXPECT_LT(descPos, authorPos);
    EXPECT_LT(authorPos, versionPos);
    EXPECT_LT(versionPos, reqVersionPos);
}

TEST(DarkmodTxt, RoundTripPreservesFields)
{
    std::string input =
        "Title: My Map\n"
        "Description: A great map\n"
        "Author: Me\n"
        "Version: 1.0\n"
        "Required TDM Version: 2.10";

    auto first = map::DarkmodTxt::CreateFromString(input);
    auto second = map::DarkmodTxt::CreateFromString(first->toString());

    EXPECT_EQ(second->getTitle(), "My Map");
    EXPECT_EQ(second->getDescription(), "A great map");
    EXPECT_EQ(second->getAuthor(), "Me");
    EXPECT_EQ(second->getVersion(), "1.0");
    EXPECT_EQ(second->getReqTdmVersion(), "2.10");
}

TEST(DarkmodTxt, RoundTripNormalisesOutOfOrderInput)
{
    std::string input =
        "Title: My Map\n"
        "Author: Me\n"
        "Description: A great map";

    auto first = map::DarkmodTxt::CreateFromString(input);
    std::string normalised = first->toString();

    EXPECT_LT(normalised.find("Description:"), normalised.find("Author:"));

    auto second = map::DarkmodTxt::CreateFromString(normalised);
    EXPECT_EQ(second->getTitle(), "My Map");
    EXPECT_EQ(second->getAuthor(), "Me");
    EXPECT_EQ(second->getDescription(), "A great map");
}

}
