#include "external/boost/ut.hpp"

#include "game.h"
#include "gamedata.h"
#include "events.h"

// Because boost::ut has it's own concept of events, as does Game, we cannot just use do
// using namespace boost::ut; here. Instead, we alias it, and then use the alias inside the
// closure to make the user defined literals and all the other niceness available.
namespace ut = boost::ut;
namespace ev = events; // these are the events from events.h

// This suite will test various aspects of the Faction class in isolation.
ut::suite<"Landmark Order"> landmark_order_suite = []
{
  using namespace ut;

  Landmark landmark1 = {
    .type = ev::SETTLEMENT,
    .name = "Settlement 1",
    .title = "Settlement 1",
    .distance = 1,
    .weight = 1,
    .x = 1,
    .y = 1,
    .z = 1
  };

  Landmark landmark2 = {
    .type = ev::SETTLEMENT,
    .name = "Settlement 2",
    .title = "Settlement 2",
    .distance = 2,
    .weight = 1,
    .x = 1,
    .y = 1,
    .z = 1
  };

  "Landmarks prefer shortest distance"_test = [landmark1, landmark2]
  {
    expect(eq(compareLandmarks(landmark1, landmark2), true));
    expect(eq(compareLandmarks(landmark2, landmark1), false));
  };

  "When distance is equal, landmarks prefer higher weight"_test = [landmark1, landmark2]() mutable
  {
    landmark2.distance = 1;
    landmark2.weight = 2;
    expect(eq(compareLandmarks(landmark1, landmark2), false));
    expect(eq(compareLandmarks(landmark2, landmark1), true));
  };

  "When distance and weight are equal, landmarks prefer lower x"_test = [landmark1, landmark2]() mutable
  {
    landmark2.distance = 1;
    landmark2.weight = 1;
    landmark2.x = 2;
    expect(eq(compareLandmarks(landmark1, landmark2), true));
    expect(eq(compareLandmarks(landmark2, landmark1), false));
  };

  "When distance, weight, and x are equal, landmarks prefer lower y"_test = [landmark1, landmark2]() mutable
  {
    landmark2.distance = 1;
    landmark2.weight = 1;
    landmark2.x = 1;
    landmark2.y = 2;
    expect(eq(compareLandmarks(landmark1, landmark2), true));
    expect(eq(compareLandmarks(landmark2, landmark1), false));
  };
};
