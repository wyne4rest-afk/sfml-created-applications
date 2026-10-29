#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <ctime>

// ═══════════════════════════════════════════════════════════════════════════════
//  RESULT STRUCTS
// ═══════════════════════════════════════════════════════════════════════════════

struct BoxResult {
    double width, height, depth;
    double cost, surfaceArea;
};

struct CylResult {
    double radius, height;
    double cost, surfaceArea;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  OPTIMIZER  –  pure math, no state
// ═══════════════════════════════════════════════════════════════════════════════

class Optimizer {
public:
    static BoxResult box(double V, double cBase, double cSide) {
        BoxResult r{};
        // Condition that if the base cost or the side cost is less than 0 it means the input is Invalid
        
        if (V <= 0 || (cBase <= 0 && cSide <= 0)) return r;
        double w = (cBase <= 0 || cSide <= 0)
                   ? std::cbrt(V)
                   : std::cbrt((cSide * V) / cBase);
        // Since you already found the width which is equal to the depth you can now compute for the rest

        double h = V / (w * w);
        r.width = r.depth = w;
        r.height      = h;
        r.surfaceArea = 2.0*w*w + 4.0*w*h;
        r.cost        = cBase*2.0*w*w + cSide*4.0*w*h;
        return r;
    }

    static CylResult cylinder(double V, double cBase, double cSide) {
        CylResult r{};
        // Condition that if the base cost of the side cost is less than 0 it means the input is Invalid

        if (V <= 0 || (cBase <= 0 && cSide <= 0)) return r;
        constexpr double PI = 3.14159265358979;
        double rad = (cBase <= 0 || cSide <= 0)
                     ? std::cbrt(V / PI)
                     : std::cbrt((cSide * V) / (2.0 * PI * cBase));

        // Since you already found the radius you can now compute for the rest of the formula
        double h = V / (PI * rad * rad);
        r.radius      = rad;
        r.height      = h;
        r.surfaceArea = 2.0*PI*rad*rad + 2.0*PI*rad*h;
        r.cost        = cBase*2.0*PI*rad*rad + cSide*2.0*PI*rad*h;
        return r;
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  RECORD MANAGER  –  all file I/O in one place
// ═══════════════════════════════════════════════════════════════════════════════

class RecordManager {
public:
    static const std::string SAVE_FILE;

    // Append one record to the file
    static void save(const std::string& shape,
                     double vol, double cBase, double cSide,
                     double p1, double p2, double p3,
                     double sa, double cost)
    {
      // This is to add 1 to the count of records there are currently in the file
        int idx = countRecords() + 1;

        // This is to seed the time into a string so that when you save a record it will have a time 
        // period to when it is saved.
         
        std::time_t now = std::time(nullptr);
        char tbuf[32];

        // This is where the copying of the date into a buffer which is called tbuf \(an abreviation of time buffer\)
        std::strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

        // This will open the file and puts the cursor to the end of the file \(hence the name std::ios::app which just
        // means append\)
        std::ofstream fout(SAVE_FILE, std::ios::app);

        // This is where the time buffer is added to the file and all the details of the box/cylinder depending on the
        // string named shape
        
        fout << "\n[#" << idx << "] " << tbuf << "\n";
        fout << "Shape       : " << shape << "\n";
        fout << "Volume      : " << std::fixed << std::setprecision(4) << vol << " cu.m\n";
        fout << "Cost Base   : PHP " << cBase << " /sq.m\n";
        fout << "Cost Side   : PHP " << cSide << " /sq.m\n";
        if (shape == "BOX") {
            fout << "Width  (w)  : " << p1 << " m\n";
            fout << "Height (h)  : " << p2 << " m\n";
            fout << "Depth  (d)  : " << p3 << " m\n";
        } else {
            fout << "Radius (r)  : " << p1 << " m\n";
            fout << "Height (h)  : " << p2 << " m\n";
        }
        fout << "Surface Area: " << sa   << " sq.m\n";
        fout << "Min Cost    : PHP " << std::setprecision(2) << cost << "\n";
        fout << "--------------------------------------------\n";
    }

    // Load all records as groups of display lines
    static std::vector<std::vector<std::string>> load() {
        std::vector<std::vector<std::string>> all;

        // This is used to point the start of the file to the variable called fin \(simply file input\)
        std::ifstream fin(SAVE_FILE);

        // Checks if there is really something open, if there is none - fin will be null which when you
        // used !fin.is_open\(\) it will become true and just return all \(which is empty this time\)
        if (!fin.is_open()) return all;
        std::string line;
        std::vector<std::string> cur;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;
            if (line.rfind("[#", 0) == 0 && !cur.empty()) {
                all.push_back(cur);
                cur.clear();
            }
            cur.push_back(line);
        }
        if (!cur.empty()) all.push_back(cur);
        return all;
    }

    // Delete record by 1-based index; rewrites file with renumbered entries
    static void remove(int delIdx) {
        auto all = load();
        std::ofstream fout(SAVE_FILE, std::ios::trunc);
        int newIdx = 1;   // renumber counter (only advances for kept records)
        int pos    = 1;   // original 1-based position of each record
        for (auto& rec : all) {
            if (rec.empty()) { ++pos; continue; }
            if (pos == delIdx) { ++pos; continue; }  // skip the deleted record
            for (size_t i = 0; i < rec.size(); ++i) {
                if (i == 0) {
                    size_t ts = rec[0].find(']');
                    std::string rest = (ts != std::string::npos)
                                       ? rec[0].substr(ts + 1) : "";
                    fout << "\n[#" << newIdx << "]" << rest << "\n";
                } else {
                    fout << rec[i] << "\n";
                }
            }
            ++newIdx;
            ++pos;
        }
    }

private:
    static int countRecords() {
        int idx = 0;
        std::ifstream fin(SAVE_FILE);
        std::string line;
        while (std::getline(fin, line))
            if (line.rfind("[#", 0) == 0) ++idx;
        return idx;
    }
};

const std::string RecordManager::SAVE_FILE = "box_records.txt";

// ═══════════════════════════════════════════════════════════════════════════════
//  APP STATE
// ═══════════════════════════════════════════════════════════════════════════════

enum class Screen { SELECT, BOX, CYLINDER, RECORDS };
enum class Field  { VOLUME, COST_BASE, COST_SIDE };

struct AppState {
    Screen      screen      = Screen::SELECT;
    Field       active      = Field::VOLUME;
    std::string volStr;
    std::string baseStr;
    std::string sideStr;
    bool        hasResult   = false;
    std::string errorMsg;
    std::string statusMsg;
    int         statusTimer = 0;
    BoxResult   boxRes{};
    CylResult   cylRes{};

    // Records screen
    std::vector<std::vector<std::string>> records;
    int         recScroll   = 0;
    int         recSelected = -1;
    bool        confirmDel  = false;
    Screen      prevScreen  = Screen::BOX;
    float       saveButtonY = 530.f;   // dynamic y of Save/Records buttons
};

// ═══════════════════════════════════════════════════════════════════════════════
//  RENDERER  –  all drawing logic grouped into one class
// ═══════════════════════════════════════════════════════════════════════════════

class Renderer {
public:
    Renderer(sf::RenderWindow& win, sf::Font& font, unsigned ww, unsigned wh)
        : win(win), font(font), WW(ww), WH(wh) {}

    // ── Primitive helpers ────────────────────────────────────────────────────

    static sf::Color hex(uint32_t h) {
        return sf::Color((h >> 16) & 0xFF, (h >> 8) & 0xFF, h & 0xFF);
    }

    void text(const std::string& str, float x, float y,
              unsigned sz, sf::Color col, bool bold = false) const {
        sf::Text t(font, str, sz);
        t.setPosition({x, y});
        t.setFillColor(col);
        if (bold) t.setStyle(sf::Text::Bold);
        win.draw(t);
    }

    void rect(float x, float y, float w, float h,
              sf::Color fill,
              sf::Color outline = {0, 0, 0, 0},
              float thick = 0.f) const {
        sf::RectangleShape r({w, h});
        r.setPosition({x, y});
        r.setFillColor(fill);
        if (thick > 0.f) { r.setOutlineColor(outline); r.setOutlineThickness(thick); }
        win.draw(r);
    }

    void field(const std::string& label, const std::string& value,
               const std::string& unit,
               float x, float y, float w, float h,
               bool active, bool hasError = false) const {
        sf::Color bg  = active ? hex(0x1E2A3A) : hex(0x111820);
        sf::Color bor = hasError ? sf::Color(220, 80, 80)
                      : active   ? hex(0x4ECDC4)
                                 : hex(0x2A3A4A);
        rect(x, y, w, h, bg, bor, active ? 2.f : 1.f);
        sf::Text lbl(font, label, 11); lbl.setPosition({x + 8.f, y + 5.f});
        lbl.setFillColor(hex(0x7A9AB5)); win.draw(lbl);
        std::string disp = value.empty() ? (active ? "|" : "-") : value + (active ? "|" : "");
        sf::Text val(font, disp, 16); val.setPosition({x + 8.f, y + h * 0.38f});
        val.setFillColor(value.empty() ? hex(0x3A5A7A) : sf::Color::White); win.draw(val);
        sf::Text u(font, unit, 11); u.setPosition({x + w - 62.f, y + h * 0.38f + 3.f});
        u.setFillColor(hex(0x4A7A6A)); win.draw(u);
    }

    // ── 3-D primitives ───────────────────────────────────────────────────────

    void isoBox(float ox, float oy, float W, float H, float D,
                sf::Color cFront, sf::Color cTop, sf::Color cSide) const {
        const float c30 = 0.866025f, s30 = 0.5f;
        auto ts = [&](float x, float y, float z) -> sf::Vector2f {
            return {ox + x*W*c30 - z*D*c30, oy - y*H + x*W*s30 + z*D*s30};
        };
        sf::Vector2f p000=ts(0,0,0),p100=ts(1,0,0),p010=ts(0,1,0),p110=ts(1,1,0);
        sf::Vector2f p001=ts(0,0,1),p101=ts(1,0,1),p011=ts(0,1,1),p111=ts(1,1,1);
        auto quad = [&](sf::Vector2f a, sf::Vector2f b, sf::Vector2f c, sf::Vector2f d, sf::Color col) {
            sf::VertexArray va(sf::PrimitiveType::TriangleFan, 4);
            va[0]={a,col}; va[1]={b,col}; va[2]={c,col}; va[3]={d,col}; win.draw(va);
        };
        quad(p001,p000,p010,p011,cSide);
        quad(p000,p100,p110,p010,cFront);
        quad(p010,p110,p111,p011,cTop);
        sf::Color cBase(cFront.r/2,cFront.g/2,cFront.b/2,cFront.a);
        quad(p000,p001,p101,p100,cBase);
        sf::Color ec(20,20,30,220);
        auto line = [&](sf::Vector2f a, sf::Vector2f b) {
            sf::VertexArray la(sf::PrimitiveType::Lines, 2);
            la[0]={a,ec}; la[1]={b,ec}; win.draw(la);
        };
        line(p000,p100); line(p100,p101); line(p101,p001); line(p001,p000);
        line(p010,p110); line(p110,p111); line(p111,p011); line(p011,p010);
        line(p000,p010); line(p100,p110); line(p101,p111); line(p001,p011);
    }

    void cylinder(float cx, float botY, float pxR, float pxH,
                  sf::Color cBody, sf::Color cTop, sf::Color cShadow) const {
        constexpr float PI = 3.14159265f;
        float ey = pxR * 0.35f; int segs = 60;
        rect(cx - pxR, botY - pxH, pxR, pxH, cShadow);
        rect(cx,       botY - pxH, pxR, pxH, cBody);
        auto ellipse = [&](sf::VertexArray& va, float cy, sf::Color col) {
            va[0] = {{cx, cy}, col};
            for (int i = 0; i <= segs; ++i) {
                float a = (float)i / segs * 2.f * PI;
                va[i+1] = {{cx + pxR*std::cos(a), cy + ey*std::sin(a)}, col};
            }
            win.draw(va);
        };
        sf::VertexArray botE(sf::PrimitiveType::TriangleFan, segs+2);
        ellipse(botE, botY, cShadow);
        sf::VertexArray topE(sf::PrimitiveType::TriangleFan, segs+2);
        ellipse(topE, botY - pxH, cTop);
        sf::Color ec(20,20,30,200);
        sf::VertexArray edges(sf::PrimitiveType::Lines, 4);
        edges[0]={{cx-pxR,botY},ec}; edges[1]={{cx-pxR,botY-pxH},ec};
        edges[2]={{cx+pxR,botY},ec}; edges[3]={{cx+pxR,botY-pxH},ec};
        win.draw(edges);
        sf::VertexArray topOut(sf::PrimitiveType::LineStrip, segs+2);
        for (int i = 0; i <= segs; ++i) {
            float a = (float)i / segs * 2.f * PI;
            topOut[i] = {{cx + pxR*std::cos(a), botY - pxH + ey*std::sin(a)}, ec};
        }
        topOut[segs+1] = topOut[0]; win.draw(topOut);
    }

    void meterStick(float cx, float topY, float pxPerMeter) const {
        float len = pxPerMeter, thick = 14.f;
        rect(cx - thick/2.f, topY, thick, len, hex(0xD4A853), hex(0x8B6914), 1.5f);
        for (int i = 0; i <= 10; ++i) {
            float ty = topY + i*len/10.f, tl = (i%5 == 0) ? thick : thick*0.5f;
            rect(cx - thick/2.f, ty - 0.75f, tl, 1.5f, hex(0x8B6914));
            if (i%2 == 0) {
                sf::Text t(font, std::to_string(i*10)+"cm", 9);
                t.setPosition({cx + thick/2.f + 4.f, ty - 7.f});
                t.setFillColor(hex(0x5C4010)); win.draw(t);
            }
        }
        sf::Text lbl(font, "1 m", 11); lbl.setPosition({cx - thick/2.f - 2.f, topY - 18.f});
        lbl.setFillColor(hex(0x8B6914)); lbl.setStyle(sf::Text::Bold); win.draw(lbl);
        for (float cap : {topY, topY + len}) {
            sf::CircleShape c(3.f); c.setOrigin({3.f, 3.f});
            c.setPosition({cx, cap}); c.setFillColor(hex(0x8B6914)); win.draw(c);
        }
    }

    // ── Full screens ─────────────────────────────────────────────────────────

    void drawSelectScreen() const {
        float cx = WW / 2.f;
        text("VolUX", cx-80, 50, 30, hex(0x4ECDC4), true);
        text("Choose a shape to optimise:", cx-115, 95, 14, hex(0x7A9AB5));

        // Box card
        float bx=cx-290.f, by=145.f, bw=230.f, bh=310.f;
        rect(bx,by,bw,bh,hex(0x0D1A28),hex(0x2A5A7A),2.f);
        text("BOX", bx+75, by+12, 20, hex(0x4ECDC4), true);
        isoBox(bx+bw/2.f-4.f, by+175.f, 52,60,52, hex(0x1A4A6A),hex(0x2A7AAA),hex(0x0F2E4A));
        text("V = w x w x h",    bx+18, by+215, 11, hex(0x3A6A8A));
        text("w=cbrt(Cs*V/Cb)",  bx+18, by+233, 11, hex(0x3A6A8A));
        rect(bx+15, by+265, bw-30, 35, hex(0x1A5A3A), hex(0x4ECDC4), 1.5f);
        text("SELECT BOX  [1]", bx+30, by+273, 13, hex(0x4ECDC4), true);

        // Cylinder card
        float ccx=cx+60.f, ccy=145.f, ccw=230.f, cch=310.f;
        rect(ccx,ccy,ccw,cch,hex(0x0D1A28),hex(0x2A4A7A),2.f);
        text("CYLINDER", ccx+45, ccy+12, 20, hex(0x6AADCC), true);
        cylinder(ccx+ccw/2.f, ccy+200, 42,90, hex(0x1A4A6A),hex(0x2A7AAA),hex(0x0F2E4A));
        text("V = pi*r^2*h",       ccx+18, ccy+215, 11, hex(0x3A6A8A));
        text("r=cbrt(Cs*V/2piCb)", ccx+18, ccy+233, 11, hex(0x3A6A8A));
        rect(ccx+15, ccy+265, ccw-30, 35, hex(0x1A3A5A), hex(0x6AADCC), 1.5f);
        text("SELECT CYL  [2]", ccx+28, ccy+273, 13, hex(0x6AADCC), true);

        // Records button
        float rbx=cx-80.f, rby=(float)WH-75.f;
        rect(rbx, rby, 160, 38, hex(0x1A1A3A), hex(0x5A5ACC), 1.5f);
        text("VIEW RECORDS  [R]", rbx+12, rby+11, 12, hex(0x8A8AFF), true);

        text("Click a card or press 1 / 2", cx-100, (float)WH-28, 11, hex(0x2A4A5A));
    }

    void drawLeftPanel(AppState& state, bool isBox) {
        rect(0, 0, 320, (float)WH, hex(0x0D1520));
        rect(10, 10, 80, 28, hex(0x1A2A3A), hex(0x2A4A5A), 1.f);
        text("< BACK [Esc]", 13, 15, 10, hex(0x5A8AAA));

        sf::Color titleCol = isBox ? hex(0x4ECDC4) : hex(0x6AADCC);
        text(isBox ? "BOX COST" : "CYLINDER COST", 30, 48, 20, titleCol, true);
        text("OPTIMISER", 30, 72, 20, sf::Color::White, true);
        text(isBox ? "Square-base closed box" : "Closed cylinder, min cost",
             30, 98, 11, hex(0x3A5A7A));

        field("VOLUME", state.volStr, "cu.m", 30, 125, 280, 50,
              state.active == Field::VOLUME,
              !state.errorMsg.empty() && state.volStr.empty());
        field(isBox ? "COST - TOP & BOTTOM FACES" : "COST - TOP & BOTTOM (circles)",
              state.baseStr, "PHP/sq.m", 30, 185, 280, 50,
              state.active == Field::COST_BASE);
        field(isBox ? "COST - SIDE FACES" : "COST - CURVED SIDE",
              state.sideStr, "PHP/sq.m", 30, 245, 280, 50,
              state.active == Field::COST_SIDE);

        rect(30, 305, 280, 38, hex(0x1A6B5A), titleCol, 1.5f);
        text("COMPUTE  [Enter]", 88, 314, 13, titleCol, true);

        if (!state.errorMsg.empty())
            text("! " + state.errorMsg, 30, 352, 11, sf::Color(220, 80, 80));
        else
            text("Tab to switch fields", 30, 352, 10, hex(0x2A4A5A));

        if (state.hasResult) {
            rect(30, 370, 280, 1.f, hex(0x1A3A4A));
            auto fmt = [](double v, int dp=4) {
                std::ostringstream ss; ss << std::fixed << std::setprecision(dp) << v;
                return ss.str();
            };
            float py = 380.f;
            text("OPTIMAL DIMENSIONS", 30, py, 11, titleCol, true); py += 18.f;

            std::vector<std::pair<std::string,std::string>> rows;
            if (isBox) {
                auto& r = state.boxRes;
                rows = {{"Width  (w)", fmt(r.width,4)  + " m"},
                        {"Height (h)", fmt(r.height,4) + " m"},
                        {"Depth  (d)", fmt(r.depth,4)  + " m"},
                        {"Surf.Area",  fmt(r.surfaceArea,4) + " sq.m"},
                        {"MIN COST",   "PHP " + fmt(r.cost,2)}};
            } else {
                auto& r = state.cylRes;
                rows = {{"Radius (r)", fmt(r.radius,4)  + " m"},
                        {"Height (h)", fmt(r.height,4)  + " m"},
                        {"Surf.Area",  fmt(r.surfaceArea,4) + " sq.m"},
                        {"MIN COST",   "PHP " + fmt(r.cost,2)}};
            }
            for (auto& row : rows) {
                bool isc = (row.first == "MIN COST");
                if (isc) py += 4.f;
                text(row.first + ":", 30, py, isc?14:12, hex(0x7A9AB5));
                text(row.second, 165, py, isc?14:12,
                     isc ? hex(0xFFD700) : sf::Color::White, isc);
                py += 20.f;
            }

            py += 6.f;
            state.saveButtonY = py;   // store so click handler uses the same y
            rect(30,  py, 134, 34, hex(0x0D2B1A), hex(0x2ECC71), 1.5f);
            text("SAVE  [S]", 42, py+9, 12, hex(0x2ECC71), true);
            rect(174, py, 136, 34, hex(0x1A1A30), hex(0x8A8AFF), 1.5f);
            text("RECORDS  [R]", 180, py+9, 12, hex(0x8A8AFF), true);
        }

        if (state.statusTimer > 0) {
            sf::Color sc = (state.statusMsg.rfind("Saved",0) == 0)
                           ? hex(0x2ECC71) : sf::Color(220,80,80);
            text(state.statusMsg, 30, (float)WH-25, 12, sc, true);
        }

        rect(320, 0, 1, (float)WH, hex(0x1A2A3A));
    }

    void drawRightPanel(AppState& state, bool isBox) const {
        float rx=340.f, rw=(float)WW-rx, cx=rx+rw/2.f;
        sf::Color titleCol = isBox ? hex(0x3A6A8A) : hex(0x2A5A8A);
        text("SCALE VISUALISATION", rx+20, 18, 13, titleCol);
        text(isBox ? "Box vs. 1-metre stick" : "Cylinder vs. 1-metre stick",
             rx+20, 36, 11, hex(0x2A4A5A));

        if (!state.hasResult) {
            text("Enter values and press Compute", cx-115, (float)WH/2-10, 13, hex(0x2A4A5A));
            return;
        }
        auto fmtN = [](double v, int dp) {
            std::ostringstream ss; ss << std::fixed << std::setprecision(dp) << v;
            return ss.str();
        };
        float anchorY = 540.f;

        if (isBox) {
            auto& r = state.boxRes;
            double maxDim = r.width; if (r.height > maxDim) maxDim = r.height;
            float pxM = std::max(20.f, std::min(460.f/(float)maxDim, 250.f));
            float iso  = pxM * 0.5f;
            float iW=(float)r.width*iso, iH=(float)r.height*iso, iD=(float)r.depth*iso;
            float bx = cx - 30.f;
            isoBox(bx, anchorY, iW, iH, iD, hex(0x1A4A6A), hex(0x2A7AAA), hex(0x0F2E4A));
            sf::Color dc = hex(0x3A7A9A);
            const float c30=0.866f, s30=0.5f;
            text("w="+fmtN(r.width,2)+"m",  bx+iW*c30*0.5f+5.f,  anchorY+iW*s30*0.5f+5.f,  11, dc);
            text("d="+fmtN(r.depth,2)+"m",  bx-iD*c30*0.5f-52.f, anchorY+iD*s30*0.5f+5.f,  11, dc);
            text("h="+fmtN(r.height,2)+"m", bx-iD*c30-62.f,       anchorY-iH*0.5f-10.f,     11, dc);
            float meterPx = std::max(40.f, std::min(pxM, 460.f));
            float stickX  = rx + rw - 80.f;
            meterStick(stickX, anchorY - meterPx, meterPx);
            for (float dx = bx+10.f; dx < stickX-10.f; dx += 8.f)
                rect(dx, anchorY, 3.f, 1.f, hex(0x2A4A5A));

            float cardX=(float)WW-225.f, cardY=58.f;
            rect(cardX,cardY,205,130,hex(0x0D1A28),hex(0x1A3A5A),1.f);
            text("RESULT SUMMARY", cardX+10, cardY+8, 11, hex(0x4ECDC4), true);
            text(fmtN(r.width,3)+" x "+fmtN(r.height,3)+" x "+fmtN(r.depth,3)+" m",
                 cardX+10, cardY+28, 11, sf::Color::White);
            text("Volume:",   cardX+10, cardY+50, 11, hex(0x7A9AB5));
            text(fmtN(r.width*r.height*r.depth,3)+" cu.m", cardX+72, cardY+50, 11, sf::Color::White);
            text("Min Cost:", cardX+10, cardY+72, 11, hex(0x7A9AB5));
            text("PHP "+fmtN(r.cost,2), cardX+72, cardY+72, 13, hex(0xFFD700), true);
            text("h/w = "+fmtN(r.height/r.width,3), cardX+10, cardY+100, 11, hex(0x3A6A8A));
            std::ostringstream sc; sc << "Scale: 1m = " << std::fixed << std::setprecision(0) << pxM << "px";
            text(sc.str(), rx+20, (float)WH-20, 11, hex(0x2A4A5A));

        } else {
            auto& r = state.cylRes;
            double maxDim = r.radius*2; if (r.height > maxDim) maxDim = r.height;
            float pxM = std::max(20.f, std::min(460.f/(float)maxDim, 250.f));
            float pxR = (float)r.radius*pxM, pxH = (float)r.height*pxM;
            float maxR = rw/2.f - 120.f;
            if (pxR > maxR) { float s=maxR/pxR; pxR*=s; pxH*=s; pxM*=s; }
            cylinder(cx-30.f, anchorY, pxR, pxH, hex(0x1A4A6A), hex(0x2A7AAA), hex(0x0F2E4A));
            sf::Color dc = hex(0x3A7A9A);
            text("r="+fmtN(r.radius,2)+"m", cx-30.f+pxR+5.f,   anchorY-pxH/2.f, 11, dc);
            text("h="+fmtN(r.height,2)+"m", cx-30.f-pxR-55.f, anchorY-pxH/2.f, 11, dc);
            sf::VertexArray dLine(sf::PrimitiveType::Lines, 2);
            dLine[0] = {{cx-30.f-pxR, anchorY-pxH-8.f}, dc};
            dLine[1] = {{cx-30.f+pxR, anchorY-pxH-8.f}, dc};
            win.draw(dLine);
            text("2r="+fmtN(r.radius*2,2)+"m", cx-30.f-18.f, anchorY-pxH-22.f, 10, dc);
            float meterPx = std::max(40.f, std::min(pxM, 460.f));
            float stickX  = rx + rw - 80.f;
            meterStick(stickX, anchorY-meterPx, meterPx);
            for (float dx = cx-30.f+pxR+10.f; dx < stickX-10.f; dx += 8.f)
                rect(dx, anchorY, 3.f, 1.f, hex(0x2A4A5A));

            float cardX=(float)WW-225.f, cardY=58.f;
            rect(cardX,cardY,205,130,hex(0x0D1A28),hex(0x1A3A5A),1.f);
            text("RESULT SUMMARY", cardX+10, cardY+8, 11, hex(0x6AADCC), true);
            constexpr double PI = 3.14159265358979;
            text("r="+fmtN(r.radius,3)+"m  h="+fmtN(r.height,3)+"m",
                 cardX+10, cardY+28, 11, sf::Color::White);
            text("Volume:",   cardX+10, cardY+50, 11, hex(0x7A9AB5));
            text(fmtN(PI*r.radius*r.radius*r.height,3)+" cu.m", cardX+72, cardY+50, 11, sf::Color::White);
            text("Min Cost:", cardX+10, cardY+72, 11, hex(0x7A9AB5));
            text("PHP "+fmtN(r.cost,2), cardX+72, cardY+72, 13, hex(0xFFD700), true);
            text("h/r = "+fmtN(r.height/r.radius,3), cardX+10, cardY+100, 11, hex(0x3A6A8A));
            std::ostringstream sc; sc << "Scale: 1m = " << std::fixed << std::setprecision(0) << pxM << "px";
            text(sc.str(), rx+20, (float)WH-20, 11, hex(0x2A4A5A));
        }
    }

    void drawRecordsScreen(AppState& state) const {
        rect(0, 0, (float)WW, (float)WH, hex(0x080D14));
        rect(0, 0, (float)WW, 55, hex(0x0D1A28));
        text("SAVED BOX RECORDS", 20, 14, 20, hex(0x8A8AFF), true);
        text("box_records.txt",   20, 38, 10, hex(0x3A4A6A));
        rect((float)WW-120, 10, 110, 34, hex(0x1A2A3A), hex(0x5A8AAA), 1.f);
        text("< BACK  [Esc]", (float)WW-115, 18, 11, hex(0x5A8AAA));

        auto& recs = state.records;
        if (recs.empty()) {
            text("No records saved yet.", 30, (float)WH/2-10, 15, hex(0x3A5A7A));
            text("Compute a box result and press SAVE.", 30, (float)WH/2+15, 12, hex(0x2A4A5A));
            return;
        }

        float cardH=115.f, cardPad=10.f, startY=65.f;
        float viewH = (float)WH - startY - 50.f;
        int visCount = (int)(viewH / (cardH + cardPad));
        int total    = (int)recs.size();

        state.recScroll = std::max(0, std::min(state.recScroll, total - visCount));
        if (state.recScroll < 0) state.recScroll = 0;

        for (int i = 0; i < visCount && (i + state.recScroll) < total; ++i) {
            int ridx = i + state.recScroll;
            auto& rec = recs[ridx];
            float cy = startY + i*(cardH + cardPad);
            bool sel = (state.recSelected == ridx);

            sf::Color cardBg  = sel ? hex(0x1A1230) : hex(0x0D1525);
            sf::Color cardBor = sel ? hex(0xFF5555)  : hex(0x1A3A5A);
            rect(10, cy, (float)WW-20, cardH, cardBg, cardBor, sel ? 2.f : 1.f);

            float lx=20.f, ly=cy+8.f;
            for (size_t li = 0; li < rec.size() && li < 8; ++li) {
                bool isHeader = (li == 0);
                bool isCost   = (rec[li].rfind("Min Cost", 0) == 0);
                if (rec[li].find("----") == 0) continue;
                sf::Color col = isHeader ? hex(0x8A8AFF)
                              : isCost   ? hex(0xFFD700)
                                         : hex(0xAABBCC);
                text(rec[li], lx, ly, isHeader ? 12u : 11u, col, isHeader);
                ly += 13.f;
            }

            float dbx=(float)WW-105.f, dby=cy+cardH/2.f-14.f;
            rect(dbx, dby, 90, 28, hex(0x2A0A0A), hex(0xFF5555), 1.5f);
            text("DELETE [D]", dbx+6, dby+7, 10, sf::Color(255,100,100), true);

            std::string badge = "#" + std::to_string(ridx+1);
            rect((float)WW-195.f, cy+cardH/2.f-10, 32, 20, hex(0x1A3A5A));
            text(badge, (float)WW-193.f, cy+cardH/2.f-8, 10, hex(0x4A8AAA), true);
        }

        if (total > visCount) {
            text("Scroll: UP/DOWN arrows  |  " + std::to_string(total) + " records total",
                 10, (float)WH-30, 11, hex(0x2A4A5A));
        } else {
            text(std::to_string(total) + " record(s) total", 10, (float)WH-30, 11, hex(0x2A4A5A));
        }

        if (state.confirmDel && state.recSelected >= 0) {
            rect((float)WW/2.f-180, (float)WH/2.f-60, 360, 120, hex(0x1A0808), hex(0xFF5555), 2.f);
            text("Delete record #" + std::to_string(state.recSelected+1) + "?",
                 (float)WW/2.f-130, (float)WH/2.f-44, 14, sf::Color::White, true);
            rect((float)WW/2.f-160, (float)WH/2.f+5, 140, 36, hex(0x2A0A0A), hex(0xFF5555), 1.5f);
            text("YES - DELETE  [Y]", (float)WW/2.f-153, (float)WH/2.f+14, 12, sf::Color(255,100,100), true);
            rect((float)WW/2.f+20,  (float)WH/2.f+5, 140, 36, hex(0x0A1A0A), hex(0x2ECC71), 1.5f);
            text("NO - CANCEL  [N]",  (float)WW/2.f+28,  (float)WH/2.f+14, 12, hex(0x2ECC71), true);
        }

        if (state.statusTimer > 0) {
            text(state.statusMsg, 10, (float)WH-48, 12, hex(0xFF5555), true);
        }
    }

private:
    sf::RenderWindow& win;
    sf::Font&         font;
    unsigned          WW, WH;
};

// ═══════════════════════════════════════════════════════════════════════════════
//  APP  –  owns the window, state, event loop
// ═══════════════════════════════════════════════════════════════════════════════

class App {
public:
    App()
        : window(sf::VideoMode({WW, WH}), "Cost Optimiser | SFML"),
          renderer(window, font, WW, WH)
    {
        window.setFramerateLimit(60);
        bool ok =
            font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")      ||
            font.openFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf") ||
            font.openFromFile("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf")        ||
            font.openFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf")                 ||
            font.openFromFile("/System/Library/Fonts/Helvetica.ttc")                  ||
            font.openFromFile("C:/Windows/Fonts/arial.ttf")                           ||
            font.openFromFile("C:/Windows/Fonts/segoeui.ttf")                         ||
            font.openFromFile("C:/Windows/Fonts/tahoma.ttf");
        if (!ok) throw std::runtime_error("Could not load any font.");
    }

    int run() {
        while (window.isOpen()) {
            if (state.statusTimer > 0) --state.statusTimer;
            handleEvents();
            render();
        }
        return 0;
    }

private:
    static constexpr unsigned WW = 1000, WH = 680;

    sf::RenderWindow window;
    sf::Font         font;
    AppState         state;
    Renderer         renderer;

    // ── Actions ──────────────────────────────────────────────────────────────

    void resetInputs() {
        state.volStr = state.baseStr = state.sideStr = "";
        state.hasResult = false;
        state.errorMsg  = "";
        state.active    = Field::VOLUME;
    }

    void tryCompute() {
        state.errorMsg = "";
        try {
            double vol  = std::stod(state.volStr);
            double base = state.baseStr.empty() ? 0.0 : std::stod(state.baseStr);
            double side = state.sideStr.empty() ? 0.0 : std::stod(state.sideStr);
            if (vol  <= 0) { state.errorMsg = "Volume must be > 0";            return; }
            if (base <  0 || side < 0) { state.errorMsg = "Costs cannot be negative"; return; }
            if (base == 0 && side == 0){ state.errorMsg = "At least one cost must be > 0"; return; }
            if (state.screen == Screen::BOX) state.boxRes = Optimizer::box(vol, base, side);
            else                             state.cylRes = Optimizer::cylinder(vol, base, side);
            state.hasResult = true;
        } catch (...) {
            state.errorMsg  = "Invalid number entered";
            state.hasResult = false;
        }
    }

    void doSave() {
        if (!state.hasResult) return;
        double vol  = std::stod(state.volStr);
        double base = state.baseStr.empty() ? 0.0 : std::stod(state.baseStr);
        double side = state.sideStr.empty() ? 0.0 : std::stod(state.sideStr);
        if (state.screen == Screen::BOX) {
            auto& r = state.boxRes;
            RecordManager::save("BOX", vol, base, side, r.width, r.height, r.depth, r.surfaceArea, r.cost);
        } else {
            auto& r = state.cylRes;
            RecordManager::save("CYLINDER", vol, base, side, r.radius, r.height, 0.0, r.surfaceArea, r.cost);
        }
        state.statusMsg   = "Saved to box_records.txt!";
        state.statusTimer = 180;
    }

    void openRecords() {
        state.prevScreen  = state.screen;
        state.records     = RecordManager::load();
        state.recScroll   = 0;
        state.recSelected = -1;
        state.confirmDel  = false;
        state.screen      = Screen::RECORDS;
    }

    // ── Event handling ───────────────────────────────────────────────────────

    void handleEvents() {
        while (const std::optional ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) { window.close(); return; }
            if (const auto* kp = ev->getIf<sf::Event::KeyPressed>())       onKey(*kp);
            if (const auto* mp = ev->getIf<sf::Event::MouseButtonPressed>()) onMouse(*mp);
            if (const auto* tc = ev->getIf<sf::Event::TextEntered>())       onText(*tc);
            if (const auto* sw = ev->getIf<sf::Event::MouseWheelScrolled>()) onScroll(*sw);
        }
    }

    void onKey(const sf::Event::KeyPressed& kp) {
        if (state.screen == Screen::RECORDS) {
            if (state.confirmDel) {
                if (kp.code == sf::Keyboard::Key::Y) {
                    RecordManager::remove(state.recSelected + 1);
                    state.records     = RecordManager::load();
                    state.recScroll   = std::max(0, (int)state.records.size()-1);
                    state.statusMsg   = "Record deleted.";
                    state.statusTimer = 150;
                    state.confirmDel  = false;
                    state.recSelected = -1;
                }
                if (kp.code == sf::Keyboard::Key::N || kp.code == sf::Keyboard::Key::Escape) {
                    state.confirmDel = false; state.recSelected = -1;
                }
            } else {
                if (kp.code == sf::Keyboard::Key::Escape) state.screen = state.prevScreen;
                if (kp.code == sf::Keyboard::Key::Up)   state.recScroll = std::max(0, state.recScroll-1);
                if (kp.code == sf::Keyboard::Key::Down) state.recScroll = std::min((int)state.records.size()-1, state.recScroll+1);
                if (kp.code == sf::Keyboard::Key::D && state.recSelected >= 0) state.confirmDel = true;
            }
        } else if (state.screen == Screen::SELECT) {
            if (kp.code==sf::Keyboard::Key::Num1||kp.code==sf::Keyboard::Key::Numpad1)
                { resetInputs(); state.screen=Screen::BOX; }
            if (kp.code==sf::Keyboard::Key::Num2||kp.code==sf::Keyboard::Key::Numpad2)
                { resetInputs(); state.screen=Screen::CYLINDER; }
            if (kp.code==sf::Keyboard::Key::R) openRecords();
            if (kp.code==sf::Keyboard::Key::Escape) window.close();
        } else {
            if (kp.code==sf::Keyboard::Key::Escape) { resetInputs(); state.screen=Screen::SELECT; }
            if (kp.code==sf::Keyboard::Key::Tab) {
                if      (state.active==Field::VOLUME)    state.active=Field::COST_BASE;
                else if (state.active==Field::COST_BASE) state.active=Field::COST_SIDE;
                else                                      state.active=Field::VOLUME;
            }
            if (kp.code==sf::Keyboard::Key::Enter) tryCompute();
            if (kp.code==sf::Keyboard::Key::S)     doSave();
            if (kp.code==sf::Keyboard::Key::R)     openRecords();
        }
    }

    void onMouse(const sf::Event::MouseButtonPressed& mp) {
        float mx=(float)mp.position.x, my=(float)mp.position.y;

        if (state.screen == Screen::RECORDS) {
            // Back button
            if (mx>=(float)WW-120&&mx<=(float)WW-10&&my>=10&&my<=44)
                state.screen=state.prevScreen;

            if (!state.confirmDel) {
                float startY=65.f, cardH=115.f, cardPad=10.f;
                float viewH=(float)WH-startY-50.f;
                int visCount=(int)(viewH/(cardH+cardPad));
                for (int i=0; i<visCount; ++i) {
                    int ridx=i+state.recScroll;
                    if (ridx>=(int)state.records.size()) break;
                    float cy=startY+i*(cardH+cardPad);
                    float dbx=(float)WW-105.f, dby=cy+cardH/2.f-14.f;
                    if (mx>=dbx&&mx<=dbx+90&&my>=dby&&my<=dby+28) {
                        state.recSelected=ridx; state.confirmDel=true;
                    } else if (mx>=10&&mx<=(float)WW-20&&my>=cy&&my<=cy+cardH) {
                        state.recSelected=(state.recSelected==ridx)?-1:ridx;
                    }
                }
            } else {
                float yhx=(float)WW/2.f-160, yhy=(float)WH/2.f+5;
                float nhx=(float)WW/2.f+20,  nhy=(float)WH/2.f+5;
                if (mx>=yhx&&mx<=yhx+140&&my>=yhy&&my<=yhy+36) {
                    RecordManager::remove(state.recSelected+1);
                    state.records     = RecordManager::load();
                    state.recScroll   = std::max(0,(int)state.records.size()-1);
                    state.statusMsg   = "Record deleted.";
                    state.statusTimer = 150;
                    state.confirmDel  = false; state.recSelected=-1;
                }
                if (mx>=nhx&&mx<=nhx+140&&my>=nhy&&my<=nhy+36)
                    { state.confirmDel=false; state.recSelected=-1; }
            }
        } else if (state.screen == Screen::SELECT) {
            float cx=WW/2.f;
            if (mx>=cx-290&&mx<=cx-60&&my>=145&&my<=455) { resetInputs(); state.screen=Screen::BOX; }
            if (mx>=cx+60 &&mx<=cx+290&&my>=145&&my<=455) { resetInputs(); state.screen=Screen::CYLINDER; }
            if (mx>=cx-80 &&mx<=cx+80 &&my>=(float)WH-75&&my<=(float)WH-37) openRecords();
        } else {
            if (mx>=10&&mx<=90&&my>=10&&my<=38)    { resetInputs(); state.screen=Screen::SELECT; }
            if (mx>=30&&mx<=310&&my>=125&&my<=175)  state.active=Field::VOLUME;
            else if(mx>=30&&mx<=310&&my>=185&&my<=235) state.active=Field::COST_BASE;
            else if(mx>=30&&mx<=310&&my>=245&&my<=295) state.active=Field::COST_SIDE;
            if (mx>=30&&mx<=310&&my>=305&&my<=343)  tryCompute();
            if (state.hasResult) {
                float by = state.saveButtonY;
                if (mx>=30&&mx<=164&&my>=by&&my<=by+34) doSave();
                if (mx>=174&&mx<=310&&my>=by&&my<=by+34) openRecords();
            }
        }
    }

    void onText(const sf::Event::TextEntered& tc) {
        if (state.screen != Screen::BOX && state.screen != Screen::CYLINDER) return;
        char ch = static_cast<char>(tc.unicode);
        std::string* tgt = nullptr;
        if      (state.active==Field::VOLUME)    tgt=&state.volStr;
        else if (state.active==Field::COST_BASE)  tgt=&state.baseStr;
        else if (state.active==Field::COST_SIDE)  tgt=&state.sideStr;
        if (tgt) {
            if (ch=='\b') { if(!tgt->empty()) tgt->pop_back(); }
            else if ((std::isdigit(ch)||ch=='.') &&
                     !(ch=='.'&&tgt->find('.')!=std::string::npos))
                *tgt += ch;
        }
    }

    void onScroll(const sf::Event::MouseWheelScrolled& sw) {
        if (state.screen != Screen::RECORDS) return;
        state.recScroll -= (int)sw.delta;
        state.recScroll  = std::max(0, std::min((int)state.records.size()-1, state.recScroll));
    }

    // ── Rendering ────────────────────────────────────────────────────────────

    void render() {
        window.clear(Renderer::hex(0x0A0F18));
        if (state.screen == Screen::SELECT) {
            renderer.drawSelectScreen();
        } else if (state.screen == Screen::RECORDS) {
            renderer.drawRecordsScreen(state);
        } else {
            bool isBox = (state.screen == Screen::BOX);
            renderer.drawLeftPanel(state, isBox);
            renderer.drawRightPanel(state, isBox);
        }
        window.display();
    }
};

// ═══════════════════════════════════════════════════════════════════════════════
//  ENTRY POINT
// ═══════════════════════════════════════════════════════════════════════════════

int main() {
    try {
        App app;
        return app.run();
    } catch (const std::exception& e) {
        return -1;
    }
}