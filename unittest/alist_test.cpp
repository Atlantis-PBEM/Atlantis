#include "external/boost/ut.hpp"

#include "alist.h"

// Because boost::ut has its own concept of events, as does Game, we cannot just do
// using namespace boost::ut; at file scope here. Instead, we alias it, and then use the
// alias inside the closures to make the user defined literals and comparators available.
namespace ut = boost::ut;

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
// AListElem is abstract (it exists only to hold a `next` pointer), so to build a list we
// need a concrete element type. This tiny payload-carrying element lets us assert on
// ordering (via `value`) without dragging in any heavy game class such as Unit or Faction.
struct TestElem : public AListElem {
    int value;
    // Add()/Insert() set `next` when an element joins a list, but initialise it
    // here too so stand-alone TestElems (used as "not in the list" strangers)
    // never carry an indeterminate pointer.
    TestElem(int v = 0) : value(v) { next = nullptr; }
};

// Convenience: read the payload of whatever First()/Next()/Get() handed back.
static int val(AListElem *e)
{
    return static_cast<TestElem *>(e)->value;
}

// ===========================================================================
// IMPORTANT — behavior-locking policy for this file.
//
// These suites assert the CURRENT behavior of AList (alist.cpp) and the
// forlist* macros (alist.h) exactly as they are today. They are a regression
// net, not a specification. Where a behavior looked surprising it was traced
// by hand and pinned deliberately (see the notes on Remove() and Empty()
// below). Do NOT "fix" alist.cpp/alist.h to make a test read more nicely: if a
// behavior here is genuinely wrong, change it in a separate, intentional
// commit and update these expectations in lockstep. Tests only describe the
// engine; they must never quietly paper over it.
// ===========================================================================


// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
ut::suite<"AList construction"> alist_construction_suite = []
{
    using namespace ut;

    "a fresh list is empty"_test = []
    {
        AList list;
        expect(list.Num() == 0_i);
        expect(list.First() == nullptr);
    };
};


// ---------------------------------------------------------------------------
// Insertion: Insert() (front) and Add() (back)
// ---------------------------------------------------------------------------
ut::suite<"AList insertion"> alist_insertion_suite = []
{
    using namespace ut;

    "Add appends to the back and grows Num"_test = []
    {
        AList list;
        list.Add(new TestElem(1));
        list.Add(new TestElem(2));
        list.Add(new TestElem(3));

        expect(list.Num() == 3_i);
        AListElem *e = list.First();
        expect(fatal(e != nullptr));
        expect(val(e) == 1_i);
        e = list.Next(e);
        expect(fatal(e != nullptr));
        expect(val(e) == 2_i);
        e = list.Next(e);
        expect(fatal(e != nullptr));
        expect(val(e) == 3_i);
        // Add() nulls the tail's next, so iteration terminates here.
        expect(list.Next(e) == nullptr);
    };

    "Insert prepends to the front and grows Num"_test = []
    {
        AList list;
        list.Insert(new TestElem(1));
        list.Insert(new TestElem(2));
        list.Insert(new TestElem(3));

        expect(list.Num() == 3_i);
        // Last inserted ends up first.
        AListElem *e = list.First();
        expect(fatal(e != nullptr));
        expect(val(e) == 3_i);
        e = list.Next(e);
        expect(val(e) == 2_i);
        e = list.Next(e);
        expect(val(e) == 1_i);
    };

    "Insert then Add keeps the tail intact"_test = []
    {
        // Insert sets lastelem only when the list was empty; this pins that a
        // subsequent Add() still finds the correct tail and appends after it.
        AList list;
        list.Insert(new TestElem(2)); // [2]
        list.Insert(new TestElem(1)); // [1,2]
        list.Add(new TestElem(3));    // [1,2,3]

        expect(list.Num() == 3_i);
        AListElem *e = list.First();
        expect(val(e) == 1_i);
        e = list.Next(e);
        expect(val(e) == 2_i);
        e = list.Next(e);
        expect(val(e) == 3_i);
        expect(list.Next(e) == nullptr);
    };

    "Add into an empty list sets both first and tail"_test = []
    {
        AList list;
        TestElem *only = new TestElem(42);
        list.Add(only);

        expect(list.Num() == 1_i);
        expect(list.First() == only);
        expect(list.Next(only) == nullptr);
    };
};


// ---------------------------------------------------------------------------
// Access: First(), Next(), Get()
// ---------------------------------------------------------------------------
ut::suite<"AList access"> alist_access_suite = []
{
    using namespace ut;

    "First returns the head, Next walks forward"_test = []
    {
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        list.Add(a);
        list.Add(b);

        expect(list.First() == a);
        expect(list.Next(a) == b);
        expect(list.Next(b) == nullptr);
    };

    "Next(nullptr) returns nullptr"_test = []
    {
        AList list;
        // Next() guards against a null argument and returns null rather than
        // dereferencing it.
        expect(list.Next(nullptr) == nullptr);
    };

    "Get finds an element that is in the list"_test = []
    {
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        list.Add(a);
        list.Add(b);

        expect(list.Get(a) == a);
        expect(list.Get(b) == b);
    };

    "Get returns null for an element not in the list"_test = []
    {
        AList list;
        list.Add(new TestElem(1));
        TestElem *stranger = new TestElem(99);

        expect(list.Get(stranger) == nullptr);
        delete stranger;
    };

    "Get on an empty list returns null"_test = []
    {
        AList list;
        TestElem *stranger = new TestElem(1);
        expect(list.Get(stranger) == nullptr);
        delete stranger;
    };
};


// ---------------------------------------------------------------------------
// Removal: Remove()
// ---------------------------------------------------------------------------
// NOTE on Remove() behavior that these tests pin deliberately:
//  * Remove() decides "is this the tail?" from e->next, clears lastelem when
//    e->next==0, and rebuilds lastelem to the real tail as it walks. Traced by
//    hand for head/middle/tail/single/absent inputs: the tail pointer stays
//    correct in every case, which the "*_then_Add appends correctly" tests
//    below lock in.
//  * Remove() does NOT null the removed element's own `next` pointer (unlike
//    Add()). The removed node is left dangling into the list. That is current
//    behavior and is pinned in a test; do not "clean it up" without intent.
ut::suite<"AList removal"> alist_removal_suite = []
{
    using namespace ut;

    "Remove(nullptr) is a no-op and returns 0"_test = []
    {
        AList list;
        list.Add(new TestElem(1));
        expect(list.Remove(nullptr) == 0_i);
        expect(list.Num() == 1_i);
    };

    "Remove the head"_test = []
    {
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        TestElem *c = new TestElem(3);
        list.Add(a); list.Add(b); list.Add(c);

        expect(list.Remove(a) == 1_c);
        expect(list.Num() == 2_i);
        expect(list.First() == b);
        expect(list.Next(b) == c);
        delete a;
    };

    "Remove a middle element"_test = []
    {
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        TestElem *c = new TestElem(3);
        list.Add(a); list.Add(b); list.Add(c);

        expect(list.Remove(b) == 1_c);
        expect(list.Num() == 2_i);
        expect(list.First() == a);
        expect(list.Next(a) == c);
        expect(list.Next(c) == nullptr);
        delete b;
    };

    "Remove the tail, then Add still appends correctly"_test = []
    {
        // This is the key lock-in for lastelem integrity: if removing the tail
        // corrupted lastelem, the following Add() would land in the wrong place
        // (or crash). Current behavior keeps the tail correct.
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        TestElem *c = new TestElem(3);
        list.Add(a); list.Add(b); list.Add(c);

        expect(list.Remove(c) == 1_c); // [1,2]
        delete c;
        expect(list.Num() == 2_i);

        TestElem *d = new TestElem(4);
        list.Add(d);                   // [1,2,4]
        expect(list.Num() == 3_i);
        expect(list.Next(b) == d);
        expect(list.Next(d) == nullptr);
    };

    "Remove the only element empties the list"_test = []
    {
        AList list;
        TestElem *a = new TestElem(1);
        list.Add(a);

        expect(list.Remove(a) == 1_c);
        expect(list.Num() == 0_i);
        expect(list.First() == nullptr);
        delete a;

        // And the emptied list is still usable: Add() rebuilds it from scratch.
        TestElem *b = new TestElem(2);
        list.Add(b);
        expect(list.Num() == 1_i);
        expect(list.First() == b);
    };

    "Remove an element not present (non-tail-looking) returns 0"_test = []
    {
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        list.Add(a); list.Add(b);

        // stranger->next is non-null, so Remove() never touches lastelem.
        TestElem *stranger = new TestElem(99);
        stranger->next = a;
        expect(list.Remove(stranger) == 0_c);
        expect(list.Num() == 2_i);

        // Tail is intact: Add() still appends after b.
        TestElem *d = new TestElem(4);
        list.Add(d);
        expect(list.Next(b) == d);
        delete stranger;
    };

    "Remove an absent tail-looking element leaves the tail usable"_test = []
    {
        // stranger->next == 0 makes Remove() clear lastelem up front, then
        // rebuild it while scanning. Pin that the real tail survives so a later
        // Add() still appends in the right spot.
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        list.Add(a); list.Add(b);

        TestElem *stranger = new TestElem(99);
        stranger->next = nullptr; // make it look like a tail to Remove()
        expect(list.Remove(stranger) == 0_c);
        expect(list.Num() == 2_i);

        TestElem *d = new TestElem(4);
        list.Add(d);                 // must land after b, not corrupt the chain
        expect(list.Next(b) == d);
        expect(list.Next(d) == nullptr);
        delete stranger;
    };

    "Remove does not null the removed element's next pointer"_test = []
    {
        // Current behavior: unlike Add(), Remove() leaves e->next dangling into
        // the (former) list. Locked in intentionally.
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        list.Add(a); list.Add(b);

        list.Remove(a);
        expect(a->next == b); // still points at b, not nulled
        delete a;
    };
};


// ---------------------------------------------------------------------------
// Clearing: DeleteAll() / Empty() / destructor
// ---------------------------------------------------------------------------
ut::suite<"AList clearing"> alist_clearing_suite = []
{
    using namespace ut;

    "DeleteAll leaves the list empty"_test = []
    {
        AList list;
        list.Add(new TestElem(1));
        list.Add(new TestElem(2));
        list.Add(new TestElem(3));

        list.DeleteAll();
        expect(list.Num() == 0_i);
        expect(list.First() == nullptr);
    };

    "DeleteAll on an already-empty list is safe"_test = []
    {
        AList list;
        list.DeleteAll();
        expect(list.Num() == 0_i);
        expect(list.First() == nullptr);
    };

    "list is reusable after DeleteAll"_test = []
    {
        AList list;
        list.Add(new TestElem(1));
        list.DeleteAll();

        TestElem *a = new TestElem(7);
        list.Add(a);
        expect(list.Num() == 1_i);
        expect(list.First() == a);
        expect(list.Next(a) == nullptr);
    };

    "Empty clears the list without deleting members"_test = []
    {
        // Empty() intentionally does NOT free the elements (they may be owned
        // elsewhere, e.g. Units). We can only observe the list side-effects
        // here: it becomes empty. The elements themselves are leaked on purpose
        // by this test, matching how Empty() is used.
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        list.Add(a);
        list.Add(b);

        list.Empty();
        expect(list.Num() == 0_i);
        expect(list.First() == nullptr);

        // Empty() severs the elements' next links as it goes.
        expect(a->next == nullptr);
        expect(b->next == nullptr);
    };
};


// ---------------------------------------------------------------------------
// Iteration macros: forlist / forlist_reuse / forlist_safe, plus NextLive
// ---------------------------------------------------------------------------
ut::suite<"AList iteration macros"> alist_iteration_suite = []
{
    using namespace ut;

    "forlist visits every element in order"_test = []
    {
        AList list;
        list.Add(new TestElem(10));
        list.Add(new TestElem(20));
        list.Add(new TestElem(30));

        int sum = 0;
        int count = 0;
        int last = -1;
        forlist(&list) {
            sum += val(elem);
            last = val(elem);
            count++;
        }
        expect(count == 3_i);
        expect(sum == 60_i);
        expect(last == 30_i); // order preserved, tail visited last
    };

    "forlist over an empty list runs zero iterations"_test = []
    {
        AList list;
        int count = 0;
        forlist(&list) {
            (void)elem;
            count++;
        }
        expect(count == 0_i);
    };

    "forlist_reuse re-iterates using the existing elem/_elem2"_test = []
    {
        AList list;
        list.Add(new TestElem(1));
        list.Add(new TestElem(2));

        int first_pass = 0;
        forlist(&list) {
            (void)elem;
            first_pass++;
        }
        // forlist_reuse depends on elem/_elem2 already being declared by the
        // preceding forlist in this scope.
        int second_pass = 0;
        forlist_reuse(&list) {
            (void)elem;
            second_pass++;
        }
        expect(first_pass == 2_i);
        expect(second_pass == 2_i);
    };

    "forlist tolerates removing and deleting the current element"_test = []
    {
        // The dominant memory idiom in the engine: iterate a list with a PLAIN
        // forlist and Remove()+delete the current element in the body (e.g.
        // economy.cpp `objects.Remove(obj)`, aregion.cpp `objects.Remove(o);
        // delete o;`). It is safe only because forlist precomputes _elem2 (the
        // next node) before running the body, so deleting `elem` never dangles
        // the loop's cursor. Pin that every element is still visited and the
        // survivors remain correctly linked.
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        TestElem *c = new TestElem(3);
        TestElem *d = new TestElem(4);
        list.Add(a); list.Add(b); list.Add(c); list.Add(d);

        int visited = 0;
        forlist(&list) {
            visited++;
            TestElem *t = static_cast<TestElem *>(elem);
            if (t->value % 2 == 0) {       // drop the even-valued nodes (b, d)
                list.Remove(elem);
                delete t;
            }
        }
        expect(visited == 4_i);            // all four were still visited
        expect(list.Num() == 2_i);
        expect(list.First() == a);         // survivors relinked as [1,3]
        expect(list.Next(a) == c);
        expect(list.Next(c) == nullptr);   // tail intact after deleting old tail d
    };

    "nested forlist iterates the inner list without disturbing the outer"_test = []
    {
        // region -> objects -> units is the most common iteration shape in the
        // engine (see RunStealOrders). Each forlist level declares its own
        // elem/_elem2, so the inner loop SHADOWS the outer `elem` for the rest
        // of the body. That is exactly why the engine captures the outer element
        // into a typed local at the top of the body (`ARegion *r = (ARegion*)
        // elem;`) and uses that, never `elem`, after a nested loop. This test
        // follows that idiom and pins that the outer loop still resumes
        // correctly and every pair is visited once.
        AList outer;
        outer.Add(new TestElem(100));
        outer.Add(new TestElem(200));

        AList inner;
        inner.Add(new TestElem(1));
        inner.Add(new TestElem(2));
        inner.Add(new TestElem(3));

        int outerCount = 0;
        int pairs = 0;
        long checksum = 0;
        forlist(&outer) {
            TestElem *oe = static_cast<TestElem *>(elem); // capture BEFORE inner loop
            int ov = oe->value;
            outerCount++;
            forlist(&inner) {
                pairs++;
                checksum += ov + val(elem);
            }
            // Use the captured local, not `elem` (which the inner forlist has
            // shadowed to null by now) - matching how the engine does it.
            expect(oe->value == ov);
        }
        expect(outerCount == 2_i);         // outer not corrupted by inner loop
        expect(pairs == 6_i);              // 2 outer x 3 inner
        // (100+1)+(100+2)+(100+3) + (200+1)+(200+2)+(200+3) = 306 + 606 = 912
        expect(checksum == 912_i);
    };

    "forlist_safe visits everything when nothing is removed"_test = []
    {
        AList list;
        list.Add(new TestElem(1));
        list.Add(new TestElem(2));
        list.Add(new TestElem(3));

        int count = 0;
        forlist_safe(&list) {
            (void)elem;
            count++;
        }
        expect(count == 3_i);
    };

    "forlist_safe tolerates removing the current element mid-iteration"_test = []
    {
        // This is the whole reason forlist_safe (and NextLive) exist: it walks a
        // snapshot copy and skips nodes that have been removed from the live
        // list. Remove every element as we visit it and confirm all were seen
        // and the list ends empty.
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        TestElem *c = new TestElem(3);
        list.Add(a); list.Add(b); list.Add(c);

        int visited = 0;
        forlist_safe(&list) {
            visited++;
            list.Remove(elem);
            delete static_cast<TestElem *>(elem);
        }
        expect(visited == 3_i);
        expect(list.Num() == 0_i);
        expect(list.First() == nullptr);
    };

    "forlist_safe skips an element removed earlier in the same pass"_test = []
    {
        // Remove the *next* element while visiting the first one; forlist_safe
        // must not hand us the already-removed node.
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        TestElem *c = new TestElem(3);
        list.Add(a); list.Add(b); list.Add(c);

        int visitedValues = 0;
        forlist_safe(&list) {
            int v = val(elem);
            visitedValues += v;
            if (v == 1) {
                // yank b (value 2) out before the loop reaches it
                list.Remove(b);
            }
        }
        // Should have visited a(1) and c(3) only; b(2) was removed first.
        expect(visitedValues == 4_i);
        delete b;
    };

    "NextLive advances to the next still-live position"_test = []
    {
        AList list;
        TestElem *a = new TestElem(1);
        TestElem *b = new TestElem(2);
        TestElem *c = new TestElem(3);
        list.Add(a); list.Add(b); list.Add(c);

        AListElem *copy[3] = { a, b, c };

        // From pos 0, the next live element is at pos 1 (b is still in the list).
        expect(list.NextLive(copy, 3, 0) == 1_i);

        // Remove b: NextLive from 0 must now skip pos 1 and land on pos 2 (c).
        list.Remove(b);
        expect(list.NextLive(copy, 3, 0) == 2_i);
        delete b;

        // From the last position, NextLive walks off the end (returns size).
        expect(list.NextLive(copy, 3, 2) == 3_i);
    };
};
