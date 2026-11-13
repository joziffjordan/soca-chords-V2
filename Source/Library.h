// Soca Chords - progression library
//
// HOW TO ADD YOUR OWN PROGRESSION
// Add one line to the kLibrary table below:
//
//   { "Display name", Maj or Min, style tags, "roman numerals" },
//
//  * Upper-case numeral = major chord, lower-case = minor chord.
//  * In Maj progressions numerals follow the major scale (I ii iii IV V vi).
//    In Min progressions they follow the natural minor scale (i III iv v VI VII).
//  * Prefix b or # to move a chord a semitone (bVII, bII ...).
//  * Suffixes: 7  maj7  9  sus4  sus2  add9  6  dim
//  * Each chord lasts one bar. Add @2 for two beats (half a bar), @8 for two bars.
//  * Style tags can be combined with | (e.g. Power | Groovy).
#pragma once

namespace soca
{
enum Mode { Maj = 0, Min = 1 };

enum Style : unsigned
{
    Power   = 1u << 0,
    Groovy  = 1u << 1,
    Bouyon  = 1u << 2,
    Dennery = 1u << 3,
    Afro    = 1u << 4,
    Jab     = 1u << 5,
    Chutney = 1u << 6,
    Zess    = 1u << 7
};

struct StyleName { unsigned flag; const char* name; };
static const StyleName kStyles[] = {
    { Power,   "Power Soca" },
    { Groovy,  "Groovy Soca" },
    { Bouyon,  "Bouyon" },
    { Dennery, "Dennery Segment" },
    { Afro,    "Afro-Soca" },
    { Jab,     "Jab Soca" },
    { Chutney, "Chutney Soca" },
    { Zess,    "Zess / Steam" },
};
static const int kNumStyles = (int) (sizeof (kStyles) / sizeof (kStyles[0]));

struct ProgressionDef
{
    const char* name;
    Mode mode;
    unsigned styles;
    const char* chords;
};

static const ProgressionDef kLibrary[] = {
    // ------------------------------------------------------------ MAJOR
    { "Road March Anthem",        Maj, Power | Groovy,           "I V vi IV" },
    { "Sunrise Fete",             Maj, Groovy | Afro,            "vi IV I V" },
    { "Carnival Tuesday",         Maj, Power | Groovy,           "I IV V IV" },
    { "Three Chord Jump",         Maj, Power | Chutney,          "I IV V V" },
    { "Savannah Stage",           Maj, Power | Groovy,           "I IV I V" },
    { "Sweet Soca",               Maj, Groovy,                   "I vi IV V" },
    { "Feteing Family",           Maj, Groovy | Afro,            "IV I V vi" },
    { "Hands In The Air",         Maj, Power | Groovy,           "I V IV IV" },
    { "Climb To The Hook",        Maj, Power | Groovy,           "IV V vi I" },
    { "Bright Morning",           Maj, Groovy,                   "I IV vi V" },
    { "Stepping Up",              Maj, Groovy,                   "I iii IV V" },
    { "Eight Bar Canon",          Maj, Groovy,                   "I V vi iii IV I IV V" },
    { "Two Five Home",            Maj, Groovy,                   "ii V I I" },
    { "Walk Up",                  Maj, Groovy | Power,           "I ii IV V" },
    { "Lift Off",                 Maj, Groovy | Power,           "IV V I vi" },
    { "Golden Hour",              Maj, Groovy | Afro,            "IV V iii vi" },
    { "Flag Wave",                Maj, Power,                    "vi V IV V" },
    { "Truck On The Road",        Maj, Power | Jab,              "I bVII IV I" },
    { "Mixolydian Stomp",         Maj, Power | Jab,              "I bVII" },
    { "Old Time Vamp",            Maj, Power | Bouyon | Chutney, "I IV" },
    { "Calypso Tent",             Maj, Power | Chutney,          "I V" },
    { "Build It Up",              Maj, Power | Groovy,           "I I IV V" },
    { "Back And Forth",           Maj, Power | Groovy,           "I V I IV" },
    { "Fast Jump And Wave",       Maj, Power,                    "I@2 IV@2 V@2 IV@2" },
    { "Fast Anthem",              Maj, Power,                    "I@2 V@2 vi@2 IV@2" },
    { "Fast Sunrise",             Maj, Power | Afro,             "vi@2 IV@2 I@2 V@2" },
    { "Fast Three Chord",         Maj, Power | Chutney,          "I@2 IV@2 V@2 V@2" },
    { "Fast Lift",                Maj, Power,                    "IV@2 V@2 vi@2 I@2" },
    { "Turnaround",               Maj, Groovy,                   "I IV ii V" },
    { "Cooler Fete",              Maj, Groovy | Afro,            "Imaj7 IVmaj7" },
    { "Smooth Sailing",           Maj, Groovy,                   "Imaj7 vi7 ii7 V7" },
    { "Boat Ride",                Maj, Groovy | Afro,            "IVmaj7 V vi7 I" },
    { "Lime By The Beach",        Maj, Groovy,                   "ii7 V7 Imaj7 IVmaj7" },
    { "Easy Descent",             Maj, Groovy,                   "IVmaj7 iii7 ii7 Imaj7" },
    { "Open Sky",                 Maj, Groovy | Afro,            "Iadd9 V vi7 IVadd9" },
    { "Suspended Wave",           Maj, Groovy | Power,           "I Vsus4 vi IV" },
    { "Soft Start",               Maj, Groovy,                   "I iii vi IV" },
    { "Doo-Wop Soca",             Maj, Groovy | Chutney,         "I vi ii V" },
    { "Crowd Response",           Maj, Power | Groovy,           "IV I V V" },
    { "Coming Down",              Maj, Power | Groovy,           "V IV I I" },
    { "Resolve It",               Maj, Groovy | Power,           "IV V I I" },
    { "Hold The Tension",         Maj, Groovy | Afro,            "vi IV V V" },
    { "Rise Again",               Maj, Power | Groovy,           "I IV V vi" },
    { "Island Hop",               Maj, Groovy | Afro,            "IV vi I V" },
    { "Stormer",                  Maj, Power | Jab,              "I bVII bVI bVII" },
    { "Rock Soca",                Maj, Power | Jab,              "I bIII IV I" },
    { "Kaiso Two Chord",          Maj, Chutney | Power,          "I V7" },
    { "Kaiso Classic",            Maj, Chutney | Groovy,         "I IV V7 I" },
    { "Extempo",                  Maj, Chutney,                  "I V7 V7 I" },
    { "Tassa Time",               Maj, Chutney,                  "I IV I V7" },
    { "Sweet Kaiso",              Maj, Chutney | Groovy,         "I vi IV V7" },
    { "Floating",                 Maj, Groovy | Afro,            "ii V" },
    { "Never Landing",            Maj, Groovy | Afro,            "IV V" },
    { "Afro Two Step",            Maj, Afro | Groovy,            "vi IV" },
    { "Bittersweet",              Maj, Groovy,                   "I iii IV iv" },
    { "Afro Lush",                Maj, Afro | Groovy,            "IVmaj7 I V vi7" },
    { "Last Lap",                 Maj, Power | Groovy,           "vi IV V I" },
    { "Six To Glory",             Maj, Power | Groovy,           "vi V I IV" },
    { "Bouyon Bright",            Maj, Bouyon,                   "I@2 IV@2" },
    { "Bouyon Party",             Maj, Bouyon | Power,           "I@2 V@2 IV@2 V@2" },

    // ------------------------------------------------------------ MINOR
    { "Minor Anthem",             Min, Power | Groovy | Afro,    "i VI III VII" },
    { "Down And Back",            Min, Power | Bouyon,           "i VII VI VII" },
    { "Hold Then Climb",          Min, Power | Groovy,           "i i VI VII" },
    { "Epic Arrival",             Min, Power | Afro,             "VI VII i i" },
    { "Minor Three Chord",        Min, Groovy | Chutney,         "i iv v i" },
    { "Minor Lift",               Min, Groovy | Power,           "i iv VII III" },
    { "Riddim Two Chord",         Min, Bouyon | Dennery | Jab,   "i VII" },
    { "Dark Vamp",                Min, Bouyon | Dennery | Afro,  "i VI" },
    { "Pressure",                 Min, Bouyon | Dennery | Zess,  "i iv" },
    { "Steam",                    Min, Dennery | Zess,           "i v" },
    { "One Chord Jab",            Min, Jab | Dennery | Bouyon,   "i" },
    { "Phrygian Jab",             Min, Dennery | Jab | Zess,     "i bII" },
    { "Jab Molassie",             Min, Jab | Dennery,            "i i bII i" },
    { "Harmonic Cry",             Min, Chutney | Groovy,         "i VI iv V" },
    { "Chutney Minor",            Min, Chutney,                  "i iv V7 i" },
    { "Chutney Two Chord",        Min, Chutney,                  "i V7" },
    { "Andalusian",               Min, Power | Chutney,          "i VII VI V" },
    { "Home Late",                Min, Groovy | Afro,            "VI III VII i" },
    { "Minor March",              Min, Power,                    "i III VII VI" },
    { "Minor Climb",              Min, Power | Groovy,           "i v VI VII" },
    { "Subdominant Start",        Min, Groovy,                   "iv VII i i" },
    { "Moody",                    Min, Groovy | Afro,            "i VI III v" },
    { "Afro Minor",               Min, Afro | Groovy,            "VI VII i III" },
    { "Circle Back",              Min, Groovy,                   "i VII iv i" },
    { "Fast Down And Back",       Min, Power | Bouyon,           "i@2 VII@2 VI@2 VII@2" },
    { "Fast Minor Anthem",        Min, Power,                    "i@2 VI@2 III@2 VII@2" },
    { "Fast Epic",                Min, Power,                    "VI@2 VII@2 i@2 i@2" },
    { "Late Night Lime",          Min, Groovy | Afro,            "i7 iv7" },
    { "After Hours",              Min, Groovy | Afro,            "i7 VImaj7" },
    { "Afro Night",               Min, Afro | Groovy,            "VImaj7 VII i7 v7" },
    { "Minor Circle",             Min, Groovy,                   "iv7 VII7 IIImaj7 VImaj7" },
    { "Minor Circle From Home",   Min, Groovy,                   "i7 iv7 VII7 IIImaj7" },
    { "Minor Power Rise",         Min, Power,                    "i III VI VII" },
    { "Minor Sweep",              Min, Power | Groovy,           "i VII III VI" },
    { "Dorian Vamp",              Min, Afro | Groovy,            "i IV" },
    { "Dorian Lush",              Min, Afro | Groovy,            "i7 IV7" },
    { "Dorian Road",              Min, Afro | Power,             "i VII IV i" },
    { "Stay Low",                 Min, Power | Groovy,           "i VI VII v" },
    { "Sinking",                  Min, Groovy | Bouyon,          "i VII VI VI" },
    { "Chutney Cadence",          Min, Chutney,                  "i iv i V7" },
    { "Bouyon Chop",              Min, Bouyon | Dennery,         "i@2 VII@2" },
    { "Bouyon Pressure",          Min, Bouyon | Dennery,         "i@2 iv@2" },
    { "Bouyon Dark",              Min, Bouyon,                   "i@2 VI@2" },
    { "Bouyon Hardcore",          Min, Bouyon | Jab,             "i@2 i@2 VI@2 VII@2" },
    { "Stepping Down",            Min, Power | Groovy,           "i VII VI v" },
    { "Minor Reply",              Min, Groovy | Zess,            "i v iv i" },
    { "Relative Start",           Min, Groovy | Afro,            "III VII i VI" },
    { "Dennery Drive",            Min, Dennery | Bouyon,         "i i VII VII" },
    { "Dennery Dark",             Min, Dennery | Zess,           "i i iv iv" },
    { "Zess Riddim",              Min, Zess | Dennery,           "i VI i VII" },
    { "Steam Riddim",             Min, Zess | Bouyon,            "i@2 v@2" },
    { "Jouvert Morning",          Min, Jab | Power,              "i VII i VII VI VII i i" },
    { "Minor Suspense",           Min, Groovy | Afro,            "i VI iv VII" },
    { "Minor Sus",                Min, Groovy | Afro,            "i VII VIadd9 VII" },

    // ------------------------------------------------------------ JAB SOCA
    { "Conch Shell Call",         Min, Jab,                      "i i i VII" },
    { "Black Oil",                Min, Jab,                      "i VII i i" },
    { "Chains On The Road",       Min, Jab | Dennery,            "i bII i VII" },
    { "Spicemas Monday",          Min, Jab | Bouyon,             "i@2 VII@2 i@2 VI@2" },
    { "Horn Section Jab",         Min, Jab | Power,              "i iv i VII" },
    { "Jab Pressure",             Min, Jab | Zess,               "i v i VII" },
    { "Jab Drone Rise",           Min, Jab,                      "i i i bII" },
    { "Jab In Major",             Maj, Jab | Power,              "I bVII I I" },
    { "Jab Stomp",                Maj, Jab,                      "I I bVII bVII" },
};
static const int kLibrarySize = (int) (sizeof (kLibrary) / sizeof (kLibrary[0]));

// ---------------------------------------------------------------- RHYTHMS
// Each rhythm is one bar of sixteen 16th-note steps: { step, length, velocity }.
// An empty pattern means "hold the chord for its whole length".
struct Hit { int step; int len; int vel; };
struct RhythmDef { const char* name; int numHits; Hit hits[16]; };

static const RhythmDef kRhythms[] = {
    { "Sustained chords", 0, {} },
    { "Soca offbeat stabs", 4, { {2,2,104}, {6,2,96}, {10,2,104}, {14,2,96} } },
    { "Power pump (8ths)", 8, { {0,1,84}, {2,1,108}, {4,1,84}, {6,1,108}, {8,1,84}, {10,1,108}, {12,1,84}, {14,1,108} } },
    { "Groovy 3-3-2", 6, { {0,3,104}, {3,3,92}, {6,2,98}, {8,3,104}, {11,3,92}, {14,2,98} } },
    { "Calypso strum", 6, { {0,3,104}, {4,2,88}, {6,2,100}, {10,2,92}, {12,2,88}, {14,2,100} } },
    { "Double skank", 6, { {2,1,104}, {3,1,84}, {6,2,98}, {10,1,104}, {11,1,84}, {14,2,98} } },
    { "Bouyon chop", 6, { {0,1,108}, {3,1,96}, {6,1,102}, {10,1,108}, {12,1,90}, {14,1,100} } },
    { "Dennery bounce", 7, { {0,1,108}, {3,1,92}, {6,1,100}, {7,1,84}, {10,1,104}, {12,1,92}, {15,1,96} } },
    { "Half-bar pads", 2, { {0,8,96}, {8,8,90} } },
};
static const int kNumRhythms = (int) (sizeof (kRhythms) / sizeof (kRhythms[0]));
} // namespace soca
