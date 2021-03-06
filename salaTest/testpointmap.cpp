// Copyright (C) 2017 Petros Koutsolampros

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "catch.hpp"
#include "salalib/mgraph.h"


TEST_CASE("Test SuperSpacePixel construction", "")
{
    const float EPSILON = 0.001;
    double spacing = 0.5;
    Point2f offset(0,0); // seems that this is always set to 0,0

    // create a new SuperSpacePixel
    // The PointMap needs the m_region variable from this
    // object as a definition of the area the grid needs to cover
    std::unique_ptr<SuperSpacePixel> spacePixel(new SuperSpacePixel("Test SuperSpacePixel"));

    SECTION( "Construct a plain SuperSpacePixel without underlying geometry" )
    {
        Point2f bottomLeft(0,0);
        Point2f topRight(2,4);

        // set m_region to the bounds
        spacePixel->m_region = QtRegion(bottomLeft, topRight);

        // check if the bounds are set correctly
        REQUIRE(spacePixel->m_region.bottom_left.x == Approx(bottomLeft.x).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.bottom_left.y == Approx(bottomLeft.y).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.top_right.x == Approx(topRight.x).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.top_right.y == Approx(topRight.y).epsilon(EPSILON));
    }

    SECTION( "Construct a SuperSpacePixel using underlying geometry" )
    {
        Point2f lineStart(0,0);
        Point2f lineEnd(2,4);

        Point2f bottomLeft(min(lineStart.x,lineEnd.x),min(lineStart.y,lineEnd.y));
        Point2f topRight(max(lineStart.x,lineEnd.x),max(lineStart.y,lineEnd.y));

        // push a SpacePixelFile in the SuperSpacePixel
        spacePixel->push_back(SpacePixelFile("Test SpacePixelGroup"));

        // push a ShapeMap in the SpacePixelFile
        spacePixel->tail().push_back(ShapeMap("Test ShapeMap"));

        // add a line to the ShapeMap
        spacePixel->tail().tail().makeLineShape(Line(lineStart, lineEnd));

        // check if the ShapeMap bounds are set correctly
        REQUIRE(spacePixel->tail().tail().getRegion().bottom_left.x == Approx(bottomLeft.x).epsilon(EPSILON));
        REQUIRE(spacePixel->tail().tail().getRegion().bottom_left.y == Approx(bottomLeft.y).epsilon(EPSILON));
        REQUIRE(spacePixel->tail().tail().getRegion().top_right.x == Approx(topRight.x).epsilon(EPSILON));
        REQUIRE(spacePixel->tail().tail().getRegion().top_right.y == Approx(topRight.y).epsilon(EPSILON));

        // SpacePixelGroup (and thus SuperSpacePixel and SpacePixelFile)
        // does not automatically grow its region when a new shapemap/file
        // is added to it therefore we have to do this externally
        spacePixel->tail().m_region = spacePixel->tail().tail().getRegion();

        // check if the SpacePixelFile bounds are set correctly
        REQUIRE(spacePixel->tail().m_region.bottom_left.x == Approx(bottomLeft.x).epsilon(EPSILON));
        REQUIRE(spacePixel->tail().m_region.bottom_left.y == Approx(bottomLeft.y).epsilon(EPSILON));
        REQUIRE(spacePixel->tail().m_region.top_right.x == Approx(topRight.x).epsilon(EPSILON));
        REQUIRE(spacePixel->tail().m_region.top_right.y == Approx(topRight.y).epsilon(EPSILON));

        spacePixel->m_region = spacePixel->tail().m_region;

        // check if the SuperSpacePixel bounds are set correctly
        REQUIRE(spacePixel->m_region.bottom_left.x == Approx(bottomLeft.x).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.bottom_left.y == Approx(bottomLeft.y).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.top_right.x == Approx(topRight.x).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.top_right.y == Approx(topRight.y).epsilon(EPSILON));
    }

    // construct a sample pointMap
    PointMap pointMap("Test PointMap");

    // assign the spacePixel
    bool spacePixelSet = pointMap.setSpacePixel(spacePixel.get());

    // check if the spacePixel was set
    REQUIRE(spacePixelSet);
}

TEST_CASE("Test grid filling", "")
{
    const float EPSILON = 0.001;
    double spacing = 0.5;
    Point2f offset(0,0); // seems that this is always set to 0,0

    // create a new SuperSpacePixel
    // The PointMap needs the m_region variable from this
    // object as a definition of the area the grid needs to cover
    std::unique_ptr<SuperSpacePixel> spacePixel(new SuperSpacePixel("Test SuperSpacePixel"));

    // Construct a plain SuperSpacePixel without underlying geometry
    {
        Point2f bottomLeft(0,0);
        Point2f topRight(2,4);

        // set m_region to the bounds
        spacePixel->m_region = QtRegion(bottomLeft, topRight);

        // check if the bounds are set correctly
        REQUIRE(spacePixel->m_region.bottom_left.x == Approx(bottomLeft.x).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.bottom_left.y == Approx(bottomLeft.y).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.top_right.x == Approx(topRight.x).epsilon(EPSILON));
        REQUIRE(spacePixel->m_region.top_right.y == Approx(topRight.y).epsilon(EPSILON));
    }

    // construct a sample pointMap
    PointMap pointMap("Test PointMap");

    // assign the spacePixel
    bool spacePixelSet = pointMap.setSpacePixel(spacePixel.get());

    // check if the spacePixel was set
    REQUIRE(spacePixelSet);

    // set the grid

    // create the grid with bounds as set above
    bool gridIsSet = pointMap.setGrid(spacing, offset);

    // check if the grid was set
    REQUIRE(gridIsSet);

    // check if the spacing is correct
    REQUIRE(spacing == pointMap.getSpacing());

    // fill the grid

    // seems like fill_type is actually connected to the
    // QDepthmapView class which is a GUI class (depthmapView.h)
    // TODO Disentangle GUI enum from pointMap.makePoints
    int fill_type = 0; // = QDepthmapView::FULLFILL

    Point2f gridBottomLeft = pointMap.getRegion().bottom_left;

    SECTION( "Check if the points are made when fill selection in a cell" )
    {
        // Check if the points are made (grid filled) when
        // the selected position is certainly in a cell
        // This calculation should make the point directly
        // at the centre of a central cell
        Point2f midPoint(gridBottomLeft.x + spacing * (floor(pointMap.getCols() * 0.5) + 0.5),
                         gridBottomLeft.y + spacing * (floor(pointMap.getRows() * 0.5) + 0.5));
        bool pointsMade = pointMap.makePoints(midPoint, fill_type);
        REQUIRE(pointsMade);

    }

    SECTION("Check if the points are made when fill selection between cells")
    {
        // Check if the points are made (grid filled) when
        // the selected position is certainly between cells
        // This calculation should make the point directly
        // at the edge of a central cell
        Point2f midPoint(gridBottomLeft.x + spacing * (floor(pointMap.getCols() * 0.5)),
                         gridBottomLeft.y + spacing * (floor(pointMap.getRows() * 0.5)));
        bool pointsMade = pointMap.makePoints(midPoint, fill_type);
        REQUIRE(pointsMade);
    }
}

// PointMap::setGrid is quite convoluted with various parameters
// affecting the result, such as the limits of the region to be
// covered (bottomLeft, topRight), the spacing and the location
// of the plan in space. For example every grid created will be
// in relation to the origin (0,0), no matter where the region
// is and the current pixel can always be calculated as if the
// origin always falls in the centre of a cell.

TEST_CASE("Quirks in grid creation - Origin always at 0", "")
{

    double spacing = 0.5;
    const float EPSILON = 0.001;
    Point2f offset(0,0); // seems that this is always set to 0,0

    Point2f bottomLeft(0,0);
    Point2f topRight(0,0);

    SECTION ("Region from origin to positive x, positive y quadrant")
    {
        spacing = 0.5;
        bottomLeft.x = 0;
        bottomLeft.y = 0;
        topRight.x = 1;
        topRight.y = 1;
    }

    SECTION ("Region away from origin to positive x, positive y quadrant")
    {
        spacing = 0.5;
        bottomLeft.x = 1;
        bottomLeft.y = 1;
        topRight.x = 2;
        topRight.y = 2;
    }

    SECTION ("Region from origin to negative x, negative y quadrant")
    {
        spacing = 0.5;
        bottomLeft.x = -1;
        bottomLeft.y = -1;
        topRight.x = 0;
        topRight.y = 0;
    }

    SECTION ("Region in all quadrants")
    {
        spacing = 0.5;
        bottomLeft.x = -1;
        bottomLeft.y = -1;
        topRight.x = 1;
        topRight.y = 1;
    }

    SECTION ("Region in positive x, positive y quadrant, non-rectangular")
    {
        spacing = 0.5;
        bottomLeft.x = 1;
        bottomLeft.y = 2;
        topRight.x = 3;
        topRight.y = 4;
    }

    SECTION ("Region in positive x, positive y quadrant, floating-point limits")
    {
        spacing = 0.5;
        bottomLeft.x = 1.1;
        bottomLeft.y = 2.2;
        topRight.x = 3.3;
        topRight.y = 4.4;
    }

    SECTION ("Region in positive x, positive y quadrant, floating-point limits")
    {
        spacing = 0.5;
        bottomLeft.x = 0.1;
        bottomLeft.y = 0.2;
        topRight.x = 0.3;
        topRight.y = 0.4;
    }

    SECTION ("Region in negative x, negative y quadrant, floating-point limits")
    {
        spacing = 0.5;
        bottomLeft.x = -0.4;
        bottomLeft.y = -0.3;
        topRight.x = -0.2;
        topRight.y = -0.1;
    }

    SECTION ("Region in all quadrants, floating-point limits")
    {
        spacing = 0.5;
        bottomLeft.x = -1.1;
        bottomLeft.y = -2.2;
        topRight.x = 3.3;
        topRight.y = 4.4;
    }

    SECTION ("Region in all quadrants, floating-point limits, smaller spacing")
    {
        spacing = 0.25;
        bottomLeft.x = 1.1;
        bottomLeft.y = 2.2;
        topRight.x = 3.3;
        topRight.y = 4.4;
    }

    std::unique_ptr<SuperSpacePixel> spacePixel(new SuperSpacePixel("Test SuperSpacePixel"));
    spacePixel->m_region = QtRegion(bottomLeft, topRight);
    PointMap pointMap("Test PointMap");
    bool spacePixelSet = pointMap.setSpacePixel(spacePixel.get());
    bool gridIsSet = pointMap.setGrid(spacing, offset);

    int bottomLeftPixelIndexX = int(floor(bottomLeft.x / spacing - 0.5)) + 1;
    int bottomLeftPixelIndexY = int(floor(bottomLeft.y / spacing - 0.5)) + 1;

    int topRightPixelIndexX = int(floor(topRight.x / spacing - 0.5)) + 1;
    int topRightPixelIndexY = int(floor(topRight.y / spacing - 0.5)) + 1;

    int numCellsX = topRightPixelIndexX - bottomLeftPixelIndexX + 1;
    int numCellsY = topRightPixelIndexY - bottomLeftPixelIndexY + 1;

    // check if the size of the grid is as expected
    REQUIRE(pointMap.getCols() == numCellsX);
    REQUIRE(pointMap.getRows() == numCellsY);

    Point2f gridBottomLeft(bottomLeftPixelIndexX * spacing - 0.5 * spacing,
                           bottomLeftPixelIndexY * spacing - 0.5 * spacing);

    // check if the bottom-left corner of the bottom-left pixel is as expected
    REQUIRE(pointMap.getRegion().bottom_left.x == Approx(gridBottomLeft.x).epsilon(EPSILON));
    REQUIRE(pointMap.getRegion().bottom_left.y == Approx(gridBottomLeft.y).epsilon(EPSILON));

    Point2f midPoint(gridBottomLeft.x + spacing * (floor(numCellsX * 0.5) + 0.5),
                      gridBottomLeft.y + spacing * (floor(numCellsY * 0.5) + 0.5));

    int fill_type = 0; // = QDepthmapView::FULLFILL

    bool pointsMade = pointMap.makePoints(midPoint, fill_type);

    // check if the grid is filled
    REQUIRE(pointsMade);
}
