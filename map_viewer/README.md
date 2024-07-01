# Developer Tool for Atlantis Map Visualization

This is a very simple map viewer, initially put together by
[Valdis Zobelia](https://github.com/valdisz).  Most, if not all, of the code in the hex.ts file comes from a Hex Map [tutorial](https://www.redblobgames.com/grids/hexagons/) and [implementation](https://www.redblobgames.com/grids/hexagons/implementation.html) written by [Amit Patel](http://www-cs-students.stanford.edu/~amitp/) and [Red Blob Games](https://www.redblobgames.com/)

## To use:
* run `<game> map hex hexmap.json`
* copy the `hexmap.json` file into this directory
* If this is your first time using this tool: run `yarn` to install everything.
* run `yarn start`
* This will spawn a webserver serving the content on port 1234
* Open a web browser to `http://localhost:1234`


## Known issues
* Terrain hexes show up as red.
  * Fix: add a color for it to the TERRAIN mapping in `index.ts`
* Maps overlap
  * Fix: add some amount to either the MAP_W or MAP_H constants.
  * Better: rewrite this code to figure out multiple levels dynamically.
* Level not showing up
  * Fix: Add a call to drawHexMap for the correct level with a good column/row
    offset.
  * Better: Rewrite this code to lay out all levels dynamically.
* The resources don't show up
  * Fix: Change the code to colorize based on the resource rather than terrain.
  * Better: Right the code to have a way to overlay things and a nice ui.
  