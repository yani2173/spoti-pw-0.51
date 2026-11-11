#include "SGLyricWords.h"
#include <math.h>
#include <string.h>

#define SG_WORD_MAX 24

typedef struct {
    const char *word;
    unsigned char theme;
} SGThemedWord;

// Base forms. Plurals, -ed, -ing, -in', -er, -est and -ly are derived below, so list an
// inflection only when it is irregular. Every entry is lowercase ASCII and at least 3 letters.
static const SGThemedWord sg_words[] = {
    {"fire", SGLyricThemeFire}, {"fiery", SGLyricThemeFire}, {"burn", SGLyricThemeFire},
    {"burnt", SGLyricThemeFire}, {"flame", SGLyricThemeFire}, {"blaze", SGLyricThemeFire},
    {"heat", SGLyricThemeFire}, {"hot", SGLyricThemeFire}, {"hotter", SGLyricThemeFire},
    {"hottest", SGLyricThemeFire}, {"hell", SGLyricThemeFire}, {"hellfire", SGLyricThemeFire},
    {"inferno", SGLyricThemeFire}, {"ember", SGLyricThemeFire}, {"ash", SGLyricThemeFire},
    {"smoke", SGLyricThemeFire}, {"lava", SGLyricThemeFire}, {"torch", SGLyricThemeFire},
    {"ignite", SGLyricThemeFire}, {"wildfire", SGLyricThemeFire}, {"bonfire", SGLyricThemeFire},
    {"campfire", SGLyricThemeFire}, {"firework", SGLyricThemeFire}, {"fever", SGLyricThemeFire},
    {"scorch", SGLyricThemeFire},

    {"ice", SGLyricThemeIce}, {"icy", SGLyricThemeIce}, {"icicle", SGLyricThemeIce},
    {"cold", SGLyricThemeIce}, {"freeze", SGLyricThemeIce}, {"froze", SGLyricThemeIce},
    {"frozen", SGLyricThemeIce}, {"frost", SGLyricThemeIce}, {"frosty", SGLyricThemeIce},
    {"frostbite", SGLyricThemeIce}, {"snow", SGLyricThemeIce}, {"snowy", SGLyricThemeIce},
    {"snowflake", SGLyricThemeIce}, {"snowfall", SGLyricThemeIce}, {"winter", SGLyricThemeIce},
    {"chill", SGLyricThemeIce}, {"chilly", SGLyricThemeIce}, {"shiver", SGLyricThemeIce},
    {"blizzard", SGLyricThemeIce}, {"glacier", SGLyricThemeIce}, {"arctic", SGLyricThemeIce},

    {"water", SGLyricThemeWater}, {"waterfall", SGLyricThemeWater}, {"underwater", SGLyricThemeWater},
    {"rain", SGLyricThemeWater}, {"rainy", SGLyricThemeWater}, {"raindrop", SGLyricThemeWater},
    {"rainfall", SGLyricThemeWater}, {"ocean", SGLyricThemeWater}, {"sea", SGLyricThemeWater},
    {"river", SGLyricThemeWater}, {"lake", SGLyricThemeWater}, {"tide", SGLyricThemeWater},
    {"tidal", SGLyricThemeWater}, {"tsunami", SGLyricThemeWater}, {"waves", SGLyricThemeWater},
    {"tears", SGLyricThemeWater}, {"teardrop", SGLyricThemeWater}, {"cry", SGLyricThemeWater},
    {"cries", SGLyricThemeWater}, {"cried", SGLyricThemeWater}, {"drown", SGLyricThemeWater},
    {"flood", SGLyricThemeWater}, {"pour", SGLyricThemeWater}, {"drip", SGLyricThemeWater},
    {"drizzle", SGLyricThemeWater}, {"splash", SGLyricThemeWater},

    {"love", SGLyricThemeLove}, {"lover", SGLyricThemeLove}, {"lovesick", SGLyricThemeLove},
    {"beloved", SGLyricThemeLove}, {"heart", SGLyricThemeLove}, {"heartbeat", SGLyricThemeLove},
    {"sweetheart", SGLyricThemeLove}, {"kiss", SGLyricThemeLove}, {"darling", SGLyricThemeLove},
    {"romance", SGLyricThemeLove}, {"romantic", SGLyricThemeLove}, {"adore", SGLyricThemeLove},
    {"valentine", SGLyricThemeLove}, {"crush", SGLyricThemeLove}, {"cupid", SGLyricThemeLove},
    {"amor", SGLyricThemeLove}, {"amour", SGLyricThemeLove}, {"xoxo", SGLyricThemeLove},

    {"shine", SGLyricThemeShine}, {"shiny", SGLyricThemeShine}, {"star", SGLyricThemeShine},
    {"starry", SGLyricThemeShine}, {"starlight", SGLyricThemeShine}, {"stardust", SGLyricThemeShine},
    {"supernova", SGLyricThemeShine}, {"galaxy", SGLyricThemeShine}, {"comet", SGLyricThemeShine},
    {"sun", SGLyricThemeShine}, {"sunny", SGLyricThemeShine}, {"sunshine", SGLyricThemeShine},
    {"sunlight", SGLyricThemeShine}, {"sunrise", SGLyricThemeShine}, {"light", SGLyricThemeShine},
    {"bright", SGLyricThemeShine}, {"gold", SGLyricThemeShine}, {"golden", SGLyricThemeShine},
    {"glow", SGLyricThemeShine}, {"gleam", SGLyricThemeShine}, {"crystal", SGLyricThemeShine}, {"sparkle", SGLyricThemeShine},
    {"glitter", SGLyricThemeShine}, {"shimmer", SGLyricThemeShine}, {"dazzle", SGLyricThemeShine},
    {"radiant", SGLyricThemeShine}, {"magic", SGLyricThemeShine}, {"halo", SGLyricThemeShine},
    {"angel", SGLyricThemeShine}, {"heaven", SGLyricThemeShine}, {"rainbow", SGLyricThemeShine},
    {"firefly", SGLyricThemeShine}, {"fireflies", SGLyricThemeShine},

    {"electric", SGLyricThemeElectric}, {"electricity", SGLyricThemeElectric},
    {"electrify", SGLyricThemeElectric}, {"thunder", SGLyricThemeElectric},
    {"thunderstorm", SGLyricThemeElectric}, {"thunderbolt", SGLyricThemeElectric},
    {"lightning", SGLyricThemeElectric}, {"lightnin", SGLyricThemeElectric},
    {"storm", SGLyricThemeElectric}, {"stormy", SGLyricThemeElectric},
    {"hurricane", SGLyricThemeElectric}, {"tornado", SGLyricThemeElectric},
    {"shock", SGLyricThemeElectric}, {"spark", SGLyricThemeElectric},
    {"power", SGLyricThemeElectric}, {"energy", SGLyricThemeElectric},
    {"volt", SGLyricThemeElectric}, {"voltage", SGLyricThemeElectric},
    {"neon", SGLyricThemeElectric}, {"laser", SGLyricThemeElectric},
    {"static", SGLyricThemeElectric}, {"buzz", SGLyricThemeElectric},

    {"night", SGLyricThemeNight}, {"tonight", SGLyricThemeNight}, {"midnight", SGLyricThemeNight},
    {"nightfall", SGLyricThemeNight}, {"nightmare", SGLyricThemeNight}, {"dark", SGLyricThemeNight},
    {"darkness", SGLyricThemeNight}, {"shadow", SGLyricThemeNight}, {"ghost", SGLyricThemeNight},
    {"phantom", SGLyricThemeNight}, {"demon", SGLyricThemeNight}, {"haunt", SGLyricThemeNight},
    {"moon", SGLyricThemeNight}, {"moonlight", SGLyricThemeNight}, {"eclipse", SGLyricThemeNight},
    {"void", SGLyricThemeNight}, {"abyss", SGLyricThemeNight}, {"gloom", SGLyricThemeNight},
    {"dusk", SGLyricThemeNight}, {"twilight", SGLyricThemeNight},

    // Extended fire vocabulary.
    {"lit", SGLyricThemeFire}, {"flare", SGLyricThemeFire}, {"fuel", SGLyricThemeFire},
    {"sizzle", SGLyricThemeFire}, {"molten", SGLyricThemeFire}, {"magma", SGLyricThemeFire},
    {"volcano", SGLyricThemeFire}, {"furnace", SGLyricThemeFire}, {"cinder", SGLyricThemeFire},
    {"kindle", SGLyricThemeFire}, {"firestorm", SGLyricThemeFire}, {"dynamite", SGLyricThemeFire},
    {"explode", SGLyricThemeFire}, {"explosion", SGLyricThemeFire}, {"bomb", SGLyricThemeFire},
    {"rocket", SGLyricThemeFire}, {"dragon", SGLyricThemeFire}, {"devil", SGLyricThemeFire},
    {"sunburn", SGLyricThemeFire}, {"heatwave", SGLyricThemeFire}, {"pyro", SGLyricThemeFire},
    {"spitfire", SGLyricThemeFire},

    // Extended ice vocabulary.
    {"cool", SGLyricThemeIce}, {"iceberg", SGLyricThemeIce}, {"icebox", SGLyricThemeIce},
    {"hail", SGLyricThemeIce}, {"sleet", SGLyricThemeIce}, {"polar", SGLyricThemeIce},
    {"igloo", SGLyricThemeIce}, {"frigid", SGLyricThemeIce}, {"glacial", SGLyricThemeIce},
    {"snowstorm", SGLyricThemeIce}, {"snowman", SGLyricThemeIce}, {"avalanche", SGLyricThemeIce},
    {"december", SGLyricThemeIce}, {"january", SGLyricThemeIce}, {"numb", SGLyricThemeIce},

    // Extended water vocabulary.
    {"blue", SGLyricThemeWater}, {"deep", SGLyricThemeWater}, {"sail", SGLyricThemeWater},
    {"swim", SGLyricThemeWater}, {"swam", SGLyricThemeWater}, {"swimming", SGLyricThemeWater},
    {"dive", SGLyricThemeWater}, {"sink", SGLyricThemeWater}, {"sank", SGLyricThemeWater},
    {"float", SGLyricThemeWater}, {"pool", SGLyricThemeWater}, {"puddle", SGLyricThemeWater},
    {"shore", SGLyricThemeWater}, {"beach", SGLyricThemeWater}, {"harbor", SGLyricThemeWater},
    {"island", SGLyricThemeWater}, {"surf", SGLyricThemeWater}, {"thirst", SGLyricThemeWater},
    {"thirsty", SGLyricThemeWater}, {"soak", SGLyricThemeWater}, {"wet", SGLyricThemeWater},
    {"misty", SGLyricThemeWater}, {"mist", SGLyricThemeWater}, {"fog", SGLyricThemeWater},
    {"foggy", SGLyricThemeWater}, {"dew", SGLyricThemeWater}, {"bubble", SGLyricThemeWater},
    {"aqua", SGLyricThemeWater}, {"mermaid", SGLyricThemeWater}, {"sailor", SGLyricThemeWater},
    {"anchor", SGLyricThemeWater}, {"shower", SGLyricThemeWater}, {"monsoon", SGLyricThemeWater},
    {"weep", SGLyricThemeWater}, {"wept", SGLyricThemeWater}, {"sob", SGLyricThemeWater},

    // Extended love vocabulary.
    {"hug", SGLyricThemeLove}, {"hugged", SGLyricThemeLove}, {"hugging", SGLyricThemeLove},
    {"cuddle", SGLyricThemeLove}, {"embrace", SGLyricThemeLove}, {"passion", SGLyricThemeLove},
    {"passionate", SGLyricThemeLove}, {"desire", SGLyricThemeLove}, {"affection", SGLyricThemeLove},
    {"romeo", SGLyricThemeLove}, {"juliet", SGLyricThemeLove}, {"honey", SGLyricThemeLove},
    {"sweetie", SGLyricThemeLove}, {"wedding", SGLyricThemeLove}, {"marry", SGLyricThemeLove},
    {"married", SGLyricThemeLove}, {"bride", SGLyricThemeLove}, {"soulmate", SGLyricThemeLove},
    {"caress", SGLyricThemeLove}, {"tender", SGLyricThemeLove}, {"cherish", SGLyricThemeLove},
    {"devotion", SGLyricThemeLove}, {"flirt", SGLyricThemeLove}, {"boyfriend", SGLyricThemeLove},
    {"girlfriend", SGLyricThemeLove}, {"lovebird", SGLyricThemeLove},
    {"heartbreak", SGLyricThemeLove}, {"heartache", SGLyricThemeLove},
    {"heartbroken", SGLyricThemeLove}, {"smooch", SGLyricThemeLove}, {"lips", SGLyricThemeLove},

    // Extended shine vocabulary.
    {"glory", SGLyricThemeShine}, {"glorious", SGLyricThemeShine}, {"flash", SGLyricThemeShine},
    {"flashlight", SGLyricThemeShine}, {"spotlight", SGLyricThemeShine},
    {"limelight", SGLyricThemeShine}, {"candle", SGLyricThemeShine},
    {"candlelight", SGLyricThemeShine}, {"lantern", SGLyricThemeShine}, {"beam", SGLyricThemeShine},
    {"ray", SGLyricThemeShine}, {"dawn", SGLyricThemeShine}, {"daylight", SGLyricThemeShine},
    {"sunset", SGLyricThemeShine}, {"sunbeam", SGLyricThemeShine}, {"aurora", SGLyricThemeShine},
    {"sparkly", SGLyricThemeShine}, {"twinkle", SGLyricThemeShine}, {"glisten", SGLyricThemeShine},
    {"glimmer", SGLyricThemeShine}, {"luminous", SGLyricThemeShine},
    {"brilliant", SGLyricThemeShine}, {"silver", SGLyricThemeShine}, {"pearl", SGLyricThemeShine},
    {"platinum", SGLyricThemeShine}, {"crown", SGLyricThemeShine}, {"treasure", SGLyricThemeShine},
    {"miracle", SGLyricThemeShine}, {"heavenly", SGLyricThemeShine}, {"divine", SGLyricThemeShine},
    {"holy", SGLyricThemeShine}, {"paradise", SGLyricThemeShine},
    {"constellation", SGLyricThemeShine}, {"cosmic", SGLyricThemeShine},
    {"cosmos", SGLyricThemeShine}, {"universe", SGLyricThemeShine}, {"planet", SGLyricThemeShine},
    {"meteor", SGLyricThemeShine}, {"nova", SGLyricThemeShine},

    // Extended electric vocabulary.
    {"zap", SGLyricThemeElectric}, {"zapped", SGLyricThemeElectric},
    {"plasma", SGLyricThemeElectric}, {"circuit", SGLyricThemeElectric},
    {"wire", SGLyricThemeElectric}, {"charge", SGLyricThemeElectric},
    {"recharge", SGLyricThemeElectric}, {"battery", SGLyricThemeElectric},
    {"surge", SGLyricThemeElectric}, {"pulse", SGLyricThemeElectric},
    {"bolt", SGLyricThemeElectric}, {"blast", SGLyricThemeElectric},
    {"burst", SGLyricThemeElectric}, {"amp", SGLyricThemeElectric},
    {"amplify", SGLyricThemeElectric}, {"overdrive", SGLyricThemeElectric},
    {"turbo", SGLyricThemeElectric}, {"nitro", SGLyricThemeElectric},
    {"dynamo", SGLyricThemeElectric}, {"magnet", SGLyricThemeElectric},
    {"magnetic", SGLyricThemeElectric}, {"glitch", SGLyricThemeElectric},
    {"robot", SGLyricThemeElectric}, {"cyber", SGLyricThemeElectric},
    {"digital", SGLyricThemeElectric}, {"techno", SGLyricThemeElectric},
    {"watt", SGLyricThemeElectric}, {"tesla", SGLyricThemeElectric},
    {"shockwave", SGLyricThemeElectric}, {"electro", SGLyricThemeElectric},
    {"adrenaline", SGLyricThemeElectric}, {"hyper", SGLyricThemeElectric},

    // Extended night vocabulary.
    {"evening", SGLyricThemeNight}, {"nighttime", SGLyricThemeNight},
    {"nocturnal", SGLyricThemeNight}, {"starless", SGLyricThemeNight}, {"lunar", SGLyricThemeNight},
    {"vampire", SGLyricThemeNight}, {"witch", SGLyricThemeNight}, {"spooky", SGLyricThemeNight},
    {"creep", SGLyricThemeNight}, {"creepy", SGLyricThemeNight}, {"grave", SGLyricThemeNight},
    {"graveyard", SGLyricThemeNight}, {"tomb", SGLyricThemeNight}, {"death", SGLyricThemeNight},
    {"coffin", SGLyricThemeNight}, {"funeral", SGLyricThemeNight}, {"reaper", SGLyricThemeNight},
    {"sinister", SGLyricThemeNight}, {"evil", SGLyricThemeNight}, {"wicked", SGLyricThemeNight},
    {"curse", SGLyricThemeNight}, {"doom", SGLyricThemeNight}, {"hollow", SGLyricThemeNight},
    {"shade", SGLyricThemeNight}, {"dim", SGLyricThemeNight}, {"blackout", SGLyricThemeNight},
    {"black", SGLyricThemeNight}, {"raven", SGLyricThemeNight}, {"crow", SGLyricThemeNight},
    {"wolf", SGLyricThemeNight}, {"wolves", SGLyricThemeNight}, {"howl", SGLyricThemeNight},
    {"owl", SGLyricThemeNight}, {"monster", SGLyricThemeNight}, {"zombie", SGLyricThemeNight},
    {"sleep", SGLyricThemeNight}, {"asleep", SGLyricThemeNight}, {"sleepy", SGLyricThemeNight},
    {"dream", SGLyricThemeNight}, {"dreamy", SGLyricThemeNight}, {"dreamt", SGLyricThemeNight},
    {"lullaby", SGLyricThemeNight}, {"slumber", SGLyricThemeNight}, {"insomnia", SGLyricThemeNight},

    // Sky.
    {"sky", SGLyricThemeSky}, {"skies", SGLyricThemeSky}, {"fly", SGLyricThemeSky},
    {"flies", SGLyricThemeSky}, {"flew", SGLyricThemeSky}, {"flown", SGLyricThemeSky},
    {"flight", SGLyricThemeSky}, {"wing", SGLyricThemeSky}, {"cloud", SGLyricThemeSky},
    {"cloudy", SGLyricThemeSky}, {"air", SGLyricThemeSky}, {"wind", SGLyricThemeSky},
    {"windy", SGLyricThemeSky}, {"breeze", SGLyricThemeSky}, {"soar", SGLyricThemeSky},
    {"feather", SGLyricThemeSky}, {"bird", SGLyricThemeSky}, {"high", SGLyricThemeSky},
    {"airplane", SGLyricThemeSky}, {"plane", SGLyricThemeSky}, {"jet", SGLyricThemeSky},
    {"parachute", SGLyricThemeSky}, {"kite", SGLyricThemeSky}, {"balloon", SGLyricThemeSky},
    {"glide", SGLyricThemeSky}, {"hover", SGLyricThemeSky}, {"altitude", SGLyricThemeSky},
    {"horizon", SGLyricThemeSky}, {"weightless", SGLyricThemeSky}, {"gravity", SGLyricThemeSky},
    {"space", SGLyricThemeSky}, {"astronaut", SGLyricThemeSky}, {"orbit", SGLyricThemeSky},

    // Money.
    {"money", SGLyricThemeMoney}, {"cash", SGLyricThemeMoney}, {"rich", SGLyricThemeMoney},
    {"dollar", SGLyricThemeMoney}, {"bank", SGLyricThemeMoney}, {"bankroll", SGLyricThemeMoney},
    {"millionaire", SGLyricThemeMoney}, {"billionaire", SGLyricThemeMoney},
    {"million", SGLyricThemeMoney}, {"billion", SGLyricThemeMoney}, {"wealth", SGLyricThemeMoney},
    {"wealthy", SGLyricThemeMoney}, {"fortune", SGLyricThemeMoney}, {"jackpot", SGLyricThemeMoney},
    {"payday", SGLyricThemeMoney}, {"paid", SGLyricThemeMoney}, {"luxury", SGLyricThemeMoney},
    {"expensive", SGLyricThemeMoney}, {"price", SGLyricThemeMoney}, {"pricey", SGLyricThemeMoney},
    {"bucks", SGLyricThemeMoney}, {"profit", SGLyricThemeMoney}, {"loot", SGLyricThemeMoney},
    {"casino", SGLyricThemeMoney}, {"greed", SGLyricThemeMoney},
    {"greedy", SGLyricThemeMoney}, {"moolah", SGLyricThemeMoney}, {"dough", SGLyricThemeMoney},

    // Bloom.
    {"flower", SGLyricThemeBloom}, {"rose", SGLyricThemeBloom}, {"garden", SGLyricThemeBloom},
    {"bloom", SGLyricThemeBloom}, {"blossom", SGLyricThemeBloom}, {"petal", SGLyricThemeBloom},
    {"daisy", SGLyricThemeBloom}, {"lily", SGLyricThemeBloom}, {"tulip", SGLyricThemeBloom},
    {"violet", SGLyricThemeBloom}, {"lavender", SGLyricThemeBloom}, {"jasmine", SGLyricThemeBloom},
    {"orchid", SGLyricThemeBloom}, {"sunflower", SGLyricThemeBloom}, {"tree", SGLyricThemeBloom},
    {"leaf", SGLyricThemeBloom}, {"leaves", SGLyricThemeBloom}, {"forest", SGLyricThemeBloom},
    {"jungle", SGLyricThemeBloom}, {"meadow", SGLyricThemeBloom}, {"grass", SGLyricThemeBloom},
    {"green", SGLyricThemeBloom}, {"spring", SGLyricThemeBloom}, {"butterfly", SGLyricThemeBloom},
    {"eden", SGLyricThemeBloom}, {"nature", SGLyricThemeBloom}, {"earth", SGLyricThemeBloom},
    {"seed", SGLyricThemeBloom}, {"grow", SGLyricThemeBloom}, {"grown", SGLyricThemeBloom},

    // Blood.
    {"blood", SGLyricThemeBlood}, {"bloody", SGLyricThemeBlood}, {"bleed", SGLyricThemeBlood},
    {"bled", SGLyricThemeBlood}, {"pain", SGLyricThemeBlood}, {"painful", SGLyricThemeBlood},
    {"hurt", SGLyricThemeBlood}, {"wound", SGLyricThemeBlood}, {"scars", SGLyricThemeBlood},
    {"knife", SGLyricThemeBlood}, {"kill", SGLyricThemeBlood}, {"murder", SGLyricThemeBlood},
    {"war", SGLyricThemeBlood}, {"rage", SGLyricThemeBlood}, {"anger", SGLyricThemeBlood},
    {"angry", SGLyricThemeBlood}, {"hate", SGLyricThemeBlood}, {"hatred", SGLyricThemeBlood},
    {"fury", SGLyricThemeBlood}, {"furious", SGLyricThemeBlood}, {"savage", SGLyricThemeBlood},
    {"die", SGLyricThemeBlood}, {"dying", SGLyricThemeBlood}, {"dead", SGLyricThemeBlood},
    {"gun", SGLyricThemeBlood}, {"bullet", SGLyricThemeBlood}, {"fight", SGLyricThemeBlood},
    {"battle", SGLyricThemeBlood}, {"revenge", SGLyricThemeBlood}, {"crimson", SGLyricThemeBlood},

    // Party.
    {"dance", SGLyricThemeParty}, {"dancer", SGLyricThemeParty}, {"party", SGLyricThemeParty},
    {"parties", SGLyricThemeParty}, {"music", SGLyricThemeParty}, {"musical", SGLyricThemeParty},
    {"song", SGLyricThemeParty}, {"sing", SGLyricThemeParty}, {"sang", SGLyricThemeParty},
    {"sung", SGLyricThemeParty}, {"melody", SGLyricThemeParty}, {"rhythm", SGLyricThemeParty},
    {"beat", SGLyricThemeParty}, {"bass", SGLyricThemeParty}, {"drum", SGLyricThemeParty},
    {"guitar", SGLyricThemeParty}, {"piano", SGLyricThemeParty}, {"disco", SGLyricThemeParty},
    {"club", SGLyricThemeParty}, {"groove", SGLyricThemeParty}, {"groovy", SGLyricThemeParty},
    {"rave", SGLyricThemeParty}, {"festival", SGLyricThemeParty}, {"celebrate", SGLyricThemeParty},
    {"celebration", SGLyricThemeParty}, {"cheers", SGLyricThemeParty},
    {"champagne", SGLyricThemeParty}, {"confetti", SGLyricThemeParty},
    {"karaoke", SGLyricThemeParty}, {"rock", SGLyricThemeParty}, {"jazz", SGLyricThemeParty},
    {"funk", SGLyricThemeParty}, {"funky", SGLyricThemeParty}, {"bounce", SGLyricThemeParty},
    {"jump", SGLyricThemeParty}, {"radio", SGLyricThemeParty}, {"stereo", SGLyricThemeParty},
    {"encore", SGLyricThemeParty}, {"chorus", SGLyricThemeParty}, {"harmony", SGLyricThemeParty},
    {"symphony", SGLyricThemeParty}, {"tune", SGLyricThemeParty}, {"anthem", SGLyricThemeParty},
    {"loud", SGLyricThemeParty}, {"noise", SGLyricThemeParty}, {"scream", SGLyricThemeParty},
    {"shout", SGLyricThemeParty}, {"clap", SGLyricThemeParty},

    // Toxic.
    {"poison", SGLyricThemeToxic}, {"poisonous", SGLyricThemeToxic}, {"toxic", SGLyricThemeToxic},
    {"venom", SGLyricThemeToxic}, {"acid", SGLyricThemeToxic}, {"drug", SGLyricThemeToxic},
    {"pill", SGLyricThemeToxic}, {"intoxicated", SGLyricThemeToxic}, {"drunk", SGLyricThemeToxic},
    {"weed", SGLyricThemeToxic}, {"snake", SGLyricThemeToxic}, {"serpent", SGLyricThemeToxic},
    {"virus", SGLyricThemeToxic}, {"infect", SGLyricThemeToxic}, {"sick", SGLyricThemeToxic},
    {"sickness", SGLyricThemeToxic}, {"disease", SGLyricThemeToxic},
    {"radioactive", SGLyricThemeToxic}, {"nuclear", SGLyricThemeToxic},
    {"chemical", SGLyricThemeToxic}, {"slime", SGLyricThemeToxic}, {"alien", SGLyricThemeToxic},
    {"mutant", SGLyricThemeToxic}, {"plague", SGLyricThemeToxic}, {"addict", SGLyricThemeToxic},
    {"addicted", SGLyricThemeToxic}, {"addiction", SGLyricThemeToxic},
    {"overdose", SGLyricThemeToxic}, {"dope", SGLyricThemeToxic}, {"whiskey", SGLyricThemeToxic},
    {"vodka", SGLyricThemeToxic}, {"tequila", SGLyricThemeToxic}, {"wine", SGLyricThemeToxic},
    {"liquor", SGLyricThemeToxic}, {"alcohol", SGLyricThemeToxic}, {"cocaine", SGLyricThemeToxic},

    // Hip-hop vocabulary, tuned on A$AP Rocky's catalog: the words his songs lean on, as
    // single keywords. No lyric lines are stored.
    {"purple", SGLyricThemePurple}, {"sizzurp", SGLyricThemePurple}, {"syrup", SGLyricThemePurple},
    {"drank", SGLyricThemePurple}, {"codeine", SGLyricThemePurple},
    {"promethazine", SGLyricThemePurple}, {"haze", SGLyricThemePurple}, {"hazy", SGLyricThemePurple},
    {"kush", SGLyricThemePurple}, {"trippy", SGLyricThemePurple}, {"lsd", SGLyricThemePurple},
    {"psychedelic", SGLyricThemePurple}, {"hallucinate", SGLyricThemePurple},
    {"hallucination", SGLyricThemePurple}, {"lucid", SGLyricThemePurple},
    {"trill", SGLyricThemePurple}, {"wavy", SGLyricThemePurple}, {"grape", SGLyricThemePurple},
    {"amethyst", SGLyricThemePurple}, {"indigo", SGLyricThemePurple},

    {"fashion", SGLyricThemeDrip}, {"designer", SGLyricThemeDrip}, {"couture", SGLyricThemeDrip},
    {"runway", SGLyricThemeDrip}, {"vogue", SGLyricThemeDrip}, {"model", SGLyricThemeDrip},
    {"style", SGLyricThemeDrip}, {"stylish", SGLyricThemeDrip}, {"swag", SGLyricThemeDrip},
    {"fresh", SGLyricThemeDrip}, {"jiggy", SGLyricThemeDrip}, {"pretty", SGLyricThemeDrip},
    {"dapper", SGLyricThemeDrip}, {"classy", SGLyricThemeDrip}, {"fancy", SGLyricThemeDrip},
    {"elegant", SGLyricThemeDrip}, {"gorgeous", SGLyricThemeDrip}, {"glam", SGLyricThemeDrip},
    {"glamour", SGLyricThemeDrip}, {"glamorous", SGLyricThemeDrip}, {"boujee", SGLyricThemeDrip},
    {"bougie", SGLyricThemeDrip}, {"fleek", SGLyricThemeDrip}, {"sauce", SGLyricThemeDrip},
    {"saucy", SGLyricThemeDrip}, {"killa", SGLyricThemeDrip}, {"flacko", SGLyricThemeDrip},
    {"rocky", SGLyricThemeDrip}, {"asap", SGLyricThemeDrip}, {"dior", SGLyricThemeDrip},
    {"prada", SGLyricThemeDrip}, {"gucci", SGLyricThemeDrip}, {"fendi", SGLyricThemeDrip},
    {"versace", SGLyricThemeDrip}, {"chanel", SGLyricThemeDrip}, {"givenchy", SGLyricThemeDrip},
    {"balenciaga", SGLyricThemeDrip}, {"margiela", SGLyricThemeDrip}, {"balmain", SGLyricThemeDrip},
    {"armani", SGLyricThemeDrip}, {"burberry", SGLyricThemeDrip}, {"vuitton", SGLyricThemeDrip},
    {"hermes", SGLyricThemeDrip}, {"goyard", SGLyricThemeDrip}, {"cartier", SGLyricThemeDrip},
    {"rolex", SGLyricThemeDrip}, {"rollie", SGLyricThemeDrip}, {"raf", SGLyricThemeDrip},
    {"vlone", SGLyricThemeDrip}, {"bape", SGLyricThemeDrip}, {"diamond", SGLyricThemeDrip},
    {"jewel", SGLyricThemeDrip}, {"jewelry", SGLyricThemeDrip}, {"bling", SGLyricThemeDrip},
    {"chain", SGLyricThemeDrip}, {"pendant", SGLyricThemeDrip}, {"necklace", SGLyricThemeDrip},
    {"bracelet", SGLyricThemeDrip}, {"carat", SGLyricThemeDrip}, {"karat", SGLyricThemeDrip},
    {"vvs", SGLyricThemeDrip}, {"silk", SGLyricThemeDrip}, {"silky", SGLyricThemeDrip},
    {"satin", SGLyricThemeDrip}, {"velvet", SGLyricThemeDrip}, {"leather", SGLyricThemeDrip},
    {"mink", SGLyricThemeDrip}, {"cashmere", SGLyricThemeDrip}, {"linen", SGLyricThemeDrip},
    {"denim", SGLyricThemeDrip}, {"sundress", SGLyricThemeDrip}, {"babushka", SGLyricThemeDrip},
    {"tailor", SGLyricThemeDrip}, {"tuxedo", SGLyricThemeDrip}, {"closet", SGLyricThemeDrip},
    {"wardrobe", SGLyricThemeDrip}, {"outfit", SGLyricThemeDrip},

    {"lord", SGLyricThemeShine}, {"god", SGLyricThemeShine}, {"praise", SGLyricThemeShine},
    {"pray", SGLyricThemeShine}, {"bless", SGLyricThemeShine}, {"blessing", SGLyricThemeShine},
    {"amen", SGLyricThemeShine}, {"church", SGLyricThemeShine}, {"gospel", SGLyricThemeShine},
    {"hallelujah", SGLyricThemeShine}, {"goldie", SGLyricThemeShine},

    {"bands", SGLyricThemeMoney}, {"racks", SGLyricThemeMoney}, {"stacks", SGLyricThemeMoney},
    {"commas", SGLyricThemeMoney}, {"guap", SGLyricThemeMoney}, {"peso", SGLyricThemeMoney},
    {"euro", SGLyricThemeMoney}, {"yen", SGLyricThemeMoney}, {"cheddar", SGLyricThemeMoney},
    {"benjamins", SGLyricThemeMoney}, {"franklins", SGLyricThemeMoney},
    {"wallet", SGLyricThemeMoney}, {"hustle", SGLyricThemeMoney}, {"baller", SGLyricThemeMoney},
    {"tycoon", SGLyricThemeMoney}, {"mogul", SGLyricThemeMoney},

    {"riot", SGLyricThemeBlood}, {"glock", SGLyricThemeBlood}, {"choppa", SGLyricThemeBlood},
    {"chopper", SGLyricThemeBlood}, {"trigger", SGLyricThemeBlood}, {"shoot", SGLyricThemeBlood},
    {"shooter", SGLyricThemeBlood}, {"shotgun", SGLyricThemeBlood}, {"pistol", SGLyricThemeBlood},
    {"rifle", SGLyricThemeBlood}, {"ammo", SGLyricThemeBlood}, {"slaughter", SGLyricThemeBlood}, {"opps", SGLyricThemeBlood},

    {"molly", SGLyricThemeToxic}, {"xanax", SGLyricThemeToxic}, {"xan", SGLyricThemeToxic},
    {"xanny", SGLyricThemeToxic}, {"percocet", SGLyricThemeToxic}, {"perc", SGLyricThemeToxic},
    {"blunt", SGLyricThemeToxic}, {"stoned", SGLyricThemeToxic}, {"faded", SGLyricThemeToxic},
    {"henny", SGLyricThemeToxic}, {"hennessy", SGLyricThemeToxic}, {"gin", SGLyricThemeToxic},
    {"bong", SGLyricThemeToxic}, {"shrooms", SGLyricThemeToxic}, {"ganja", SGLyricThemeToxic},
    {"reefer", SGLyricThemeToxic}, {"marijuana", SGLyricThemeToxic},
    {"cannabis", SGLyricThemeToxic},

    {"turnt", SGLyricThemeParty}, {"mosh", SGLyricThemeParty}, {"moshpit", SGLyricThemeParty},
    {"rowdy", SGLyricThemeParty}, {"hype", SGLyricThemeParty}, {"vibe", SGLyricThemeParty},
    {"wild", SGLyricThemeParty}, {"banger", SGLyricThemeParty}, {"speaker", SGLyricThemeParty},

    {"sandman", SGLyricThemeNight}, {"casket", SGLyricThemeNight}, {"hearse", SGLyricThemeNight},
    {"skyline", SGLyricThemeSky}, {"pilot", SGLyricThemeSky}, {"rooftop", SGLyricThemeSky},
    {"freezer", SGLyricThemeIce}, {"fridge", SGLyricThemeIce},
};

// Unrelated words which the inflection rules would otherwise reduce to a keyword.
static const char *const sg_unrelated[] = {
    "staring", "stared", "stares", "number", "numbers", "numbered", "sober", "mister", "misters",
    "liter", "liters", "dimes", "crowed", "icing", "divest", "gravely", "holes",
    "warning", "winding", "winded", "painting", "painter", "singe", "singed",
    "litter", "litters", "littered", "littering", "saucer", "saucers", "bluntly",
};

static bool unrelated(const char *word, size_t length) {
    for (size_t i = 0; i < sizeof(sg_unrelated) / sizeof(sg_unrelated[0]); i++)
        if (strlen(sg_unrelated[i]) == length && memcmp(sg_unrelated[i], word, length) == 0) return true;
    return false;
}

static SGLyricTheme lookup(const char *word, size_t length) {
    if (length < 3) return SGLyricThemeNone;
    for (size_t i = 0; i < sizeof(sg_words) / sizeof(sg_words[0]); i++) {
        const char *known = sg_words[i].word;
        if (known[0] == word[0] && strlen(known) == length && memcmp(known, word, length) == 0)
            return (SGLyricTheme)sg_words[i].theme;
    }
    return SGLyricThemeNone;
}

static bool endsWith(const char *word, size_t length, const char *suffix) {
    size_t size = strlen(suffix);
    return length > size && memcmp(word + length - size, suffix, size) == 0;
}

// Tries the word with `drop` trailing characters removed, and optionally with a restored "e"
// or without a doubled final consonant.
static SGLyricTheme stem(const char *word, size_t length, size_t drop, bool restoreE) {
    char base[SG_WORD_MAX + 1];
    size_t size = length - drop;
    memcpy(base, word, size);
    SGLyricTheme theme = lookup(base, size);
    if (theme != SGLyricThemeNone || !restoreE) return theme;
    base[size] = 'e';
    if ((theme = lookup(base, size + 1)) != SGLyricThemeNone) return theme;
    // A doubled final consonant: "drippin'", "swagger", "clubbing".
    return size >= 4 && base[size - 1] == base[size - 2] ? lookup(base, size - 1) : SGLyricThemeNone;
}

static SGLyricTheme classify(const char *word, size_t length) {
    // A possessive or a dropped final letter: "heart's", "burnin'".
    if (length >= 2 && word[length - 2] == '\'' && word[length - 1] == 's') length -= 2;
    while (length && word[length - 1] == '\'') length--;
    SGLyricTheme theme = lookup(word, length);
    if (theme != SGLyricThemeNone || unrelated(word, length)) return theme;
    if (endsWith(word, length, "s") && !endsWith(word, length, "ss") &&
        (theme = stem(word, length, 1, false)) != SGLyricThemeNone) return theme;
    if (endsWith(word, length, "es") && (theme = stem(word, length, 2, false)) != SGLyricThemeNone) return theme;
    if (endsWith(word, length, "ed") && (theme = stem(word, length, 2, true)) != SGLyricThemeNone) return theme;
    if (endsWith(word, length, "ing") && (theme = stem(word, length, 3, true)) != SGLyricThemeNone) return theme;
    if (endsWith(word, length, "in") && (theme = stem(word, length, 2, true)) != SGLyricThemeNone) return theme;
    if (endsWith(word, length, "er") && (theme = stem(word, length, 2, true)) != SGLyricThemeNone) return theme;
    if (endsWith(word, length, "est") && (theme = stem(word, length, 3, true)) != SGLyricThemeNone) return theme;
    if (endsWith(word, length, "ly")) return stem(word, length, 2, false);
    return SGLyricThemeNone;
}

SGLyricTheme SGLyricThemeForWord(const char *utf8, size_t length) {
    if (!utf8) return SGLyricThemeNone;
    char word[SG_WORD_MAX + 1];
    size_t size = 0;
    SGLyricTheme theme = SGLyricThemeNone;
    for (size_t i = 0; i <= length; i++) {
        unsigned char byte = i < length ? (unsigned char)utf8[i] : ' ';
        bool apostrophe = byte == '\'';
        unsigned char next = i + 1 < length ? (unsigned char)utf8[i + 1] : 0;
        unsigned char last = i + 2 < length ? (unsigned char)utf8[i + 2] : 0;
        if (byte == 0xE2 && (next == 0x80 || next == 0x81) && last >= 0x80 && last <= 0xBF) {
            // General punctuation: curly quotes, dashes, the ellipsis, thin and zero-width spaces.
            apostrophe = next == 0x80 && (last == 0x98 || last == 0x99);
            byte = apostrophe ? '\'' : ' ';
            i += 2;
        } else if (byte == 0xC2 && next >= 0x80 && next <= 0xBF) {
            byte = ' '; // U+0080...U+00BF: a no-break space or Latin-1 punctuation, never a letter.
            i += 1;
        } else if (byte == 0xEF && next == 0xBB && last == 0xBF) {
            byte = ' '; // A byte-order mark / zero-width no-break space.
            i += 2;
        } else if (byte >= 0x80) {
            return SGLyricThemeNone; // Another script or an accented letter: not an English keyword.
        }
        // Stylized spellings: "A$AP", "L$D", "ca$h". A leading or trailing sign stays a separator.
        if (byte == '$' && size && ((next | 0x20) >= 'a' && (next | 0x20) <= 'z')) byte = 's';
        if (byte >= 'A' && byte <= 'Z') byte = (unsigned char)(byte - 'A' + 'a');
        if ((byte >= 'a' && byte <= 'z') || (apostrophe && size)) {
            if (size == SG_WORD_MAX) return SGLyricThemeNone;
            word[size++] = (char)byte;
            continue;
        }
        // Any other character ends a part: "ice-cold" and "rock&roll" are checked part by part.
        if (size && theme == SGLyricThemeNone) theme = classify(word, size);
        size = 0;
    }
    return theme;
}

static double unit(double value) {
    return value < 0.0 ? 0.0 : (value > 1.0 ? 1.0 : value);
}

SGLyricPhase SGLyricPhaseAt(double now, int64_t start, int64_t end) {
    SGLyricPhase phase = {true, false, false, 0.0f, 0.0, 0.0};
    if (!isfinite(now)) return phase;
    double from = (double)start, to = (double)end;
    if (!(to > from)) to = from + 1500.0;
    phase.upcoming = now < from;
    phase.painted = now >= from - 200.0;
    phase.emitting = now >= from && now < to;
    // The same shape as the core's own word glow: ease-out rise, smoothstep fall.
    double rise = unit((now - from) / 150.0), fall = unit((now - to) / 450.0);
    phase.glow = (float)(rise * (2.0 - rise) * (1.0 - fall * fall * (3.0 - 2.0 * fall)));
    if (phase.emitting) {
        phase.remaining = (to - now) / 1000.0;
        double seconds = phase.remaining * 2.0 + 0.5;
        phase.safety = seconds < 0.8 ? 0.8 : (seconds > 8.0 ? 8.0 : seconds);
    }
    return phase;
}
