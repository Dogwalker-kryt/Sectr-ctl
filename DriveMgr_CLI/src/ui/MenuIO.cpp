#include "../ui/MenuIO.hpp"


int MainMenuIO::colorTuiMenu(const std::vector<std::pair<MenuOptionsMain, std::string>> &menuItems, std::string &version) {
    term.enableRawMode();

    int selected = 0;
    int total = (int)menuItems.size();

    scf::array<scf::str32, 3> header_names = {
        "SECTR_CTL", "SECTR_CTL (debug)", "SECTR_CTL (stand alone)"
    };

    scf::str32 header_name = header_names[0];

    if (Globals::g_debug == true) { header_name = header_names[1]; }
    else if (Globals::stand_alone == true) { header_name = header_names[2]; }

    const size_t header_name_len = header_name.length();
    const size_t version_len = version.length();
    const size_t free_space = 46; // free white space between left border and right border 
    
    const size_t total_header_len = header_name_len + version_len;
    const float left_padding = (free_space - total_header_len) / 2;
    const float right_padding = (free_space - left_padding) - total_header_len; // + 4, for the right border to alling with the box

    scf::str32 sleft_padding(left_padding, ' ');
    scf::str32 sright_padding(right_padding, ' ');
    scf::str64 header_content = header_name + " " + scf::to_str32(version); 

    scf::print_flush("\033[2J\033[H");
    scf::println("Use Up/Down arrows and Enter to select an option.\n");
    scf::println(Globals::g_THEME_COLOR, "┌─────────────────────────────────────────────────┐", RESET);
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, BOLD, sleft_padding, header_content, sright_padding, RESET, Globals::g_THEME_COLOR, " │", RESET);
    scf::println(Globals::g_THEME_COLOR, "├─────────────────────────────────────────────────┤", RESET);
    for (size_t i = 0; i < menuItems.size(); ++i) {

        std::cout << Globals::g_THEME_COLOR << "│ " << RESET;

        // Build inner content with fixed width
        std::ostringstream inner;
        inner << std::setw(2) << menuItems[i].first << ". " << std::left << std::setw(43) << menuItems[i].second;
        std::string innerStr = inner.str();

        if (menuItems[i].first == 0) {
            innerStr = scf::to_std_str(Globals::g_THEME_COLOR) + innerStr + RESET;
        }

        // Print right border and newline
        std::cout << Globals::g_THEME_COLOR << " │\n" << RESET;

    }
    std::cout  << Globals::g_THEME_COLOR << "└─────────────────────────────────────────────────┘\n" << RESET;

    std::cout << "\033[" << (total + 1) << "A";

    while (true) {

        for (int i = 0; i < total; i++) {
            std::cout << "\r"; 

            // Build inner content
            std::ostringstream inner;
            inner << std::setw(2) << menuItems[i].first << ". "
                << std::left << std::setw(43) << menuItems[i].second;

            std::string innerStr = inner.str();

            if (menuItems[i].first == 0) {
                innerStr = scf::to_std_str(Globals::g_THEME_COLOR) + innerStr + RESET;
            }

            std::cout << Globals::g_THEME_COLOR << "│ " << RESET;

            // Apply inverse highlight if selected
            if (i == selected) std::cout << INVERSE;
            std::cout << innerStr;
            if (i == selected) std::cout << RESET;

            std::cout << Globals::g_THEME_COLOR << " │" << RESET << "\n";
        }

        // Move cursor back up to top of menu
        std::cout << "\033[" << total << "A";

        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) continue;

        if (c == '\x1b') {
            char seq[2];
            if (read(STDIN_FILENO, &seq, 2) == 2) {
                if (seq[1] == 'A') selected = (selected - 1 + total) % total;
                if (seq[1] == 'B') selected = (selected + 1) % total;
            }
        }
        else if (c == '\n' || c == '\r') {
            break;
        }
    }

    // Move cursor down past menu
    std::cout << "\033[" << (total + 1) << "B\n";

    term.restoreTerminal();
    return selected;
}

int MainMenuIO::noColorTuiMenu(const std::vector<std::pair<MenuOptionsMain, std::string>> &menuItems, std::string &version) {
    term.enableRawMode();

    int selected = 0;
    int total = (int)menuItems.size();

    scf::array<scf::str32, 3> header_names = {
        "SECTR_CTL", "SECTR_CTL (debug)", "SECTR_CTL (stand alone)"
    };

    scf::str32 header_name = header_names[0];

    if (Globals::g_debug == true) { header_name = header_names[1]; }
    else if (Globals::stand_alone == true) { header_name = header_names[2]; }

    const size_t header_name_len = header_name.length();
    const size_t version_len = version.length();
    const size_t free_space = 46; // free white space between left border and right border 
    
    const size_t total_header_len = header_name_len + version_len;
    const float left_padding = (free_space - total_header_len) / 2;
    const float right_padding = (free_space - left_padding) - total_header_len + 4; // + 4, for the right border to alling with the box

    scf::str32 sleft_padding(left_padding, ' ');
    scf::str32 sright_padding(right_padding, ' ');
    scf::str64 header_content = header_name + " " + scf::to_str32(version); 

    scf::print_flush("\033[2J\033[H");
    scf::println("Use Up/Down arrows and Enter to select an option.\n");
    scf::println("┌─────────────────────────────────────────────────────┐");
    scf::println("│ ", BOLD, sleft_padding, header_content, sright_padding, RESET, " │");
    scf::println("├─────────────────────────────────────────────────────┤");

    for (size_t i = 0; i < menuItems.size(); ++i) {

        std::cout << "│ ";

        // Build inner content with fixed width
        std::ostringstream inner;
        inner << std::setw(2) << menuItems[i].first << ". "
            << std::left << std::setw(44) << menuItems[i].second;

        std::cout << inner.str();

        std::cout << "  │\n";
    }

    std::cout << "└─────────────────────────────────────────────────────┘\n";

    // Move cursor UP to where the first selectable line is
    std::cout << "\033[" << (total + 1) << "A";

    while (true) {
        // Redraw selector arrows
        for (int i = 0; i < total; i++) {
            std::cout << "\r"; // go to start of line

            if (i == selected) std::cout << "│ "<< BOLD << "> ";
            else std::cout << "│   ";

            std::ostringstream inner;
            inner << std::setw(2) << menuItems[i].first << ". "
                    << std::left << std::setw(44) << menuItems[i].second;

            std::cout << inner.str() << "  │" << RESET << "\n";
        }

        // Move cursor back up to top of menu
        std::cout << "\033[" << total << "A";

        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) continue;

        if (c == '\x1b') {
            char seq[2];
            if (read(STDIN_FILENO, &seq, 2) == 2) {
                if (seq[1] == 'A') selected = (selected - 1 + total) % total; // up
                if (seq[1] == 'B') selected = (selected + 1) % total;         // down
            }
        } else if (c == '\n' || c == '\r') {
            break;
        }
    }

    // Move cursor down past menu
    std::cout << "\033[" << (total + 1) << "B\n";

    term.restoreTerminal();
    return selected;
}



void GenericMenuIO::printMenuLine(bool selected, const std::pair<int,std::string>& item, size_t inner_width) {
    std::string num = std::to_string(item.first) + ". ";
    size_t item_len = num.length() + item.second.length();
    size_t padding  = (inner_width > item_len) ? (inner_width - item_len) : 0;

    std::cout << Globals::g_THEME_COLOR << "│ " << RESET << (selected ? BOLD : RESET) << (selected ? "> " : "  ") 
            << num
            << item.second
            << std::string(padding, ' ')
            << RESET
            << Globals::g_THEME_COLOR
            << " │\n"
            << RESET;
}

size_t GenericMenuIO::computeInnerWidth(const std::string& title, const std::vector<std::pair<int,std::string>>& items) {
    size_t longest = title.length();

    for (const auto& item : items) {

        size_t len = std::to_string(item.first).length() + 2 + item.second.length();
        longest = std::max(longest, len);

    }

    return longest;
}

void GenericMenuIO::printDash(size_t n) {
    while (n--) std::cout << (Globals::g_no_color ? "" : Globals::g_THEME_COLOR) << "─";
    scf::print(RESET);
}

int GenericMenuIO::noColorTuiMenu(const std::string &title, const std::vector<std::pair<int, std::string>> &menuItems) {
    term.enableRawMode();

    int selected = 0;
    int total = menuItems.size();

    // Width calculations
    size_t inner_width = computeInnerWidth(title, menuItems);
    size_t box_width   = inner_width + 4;

    // Title with padding
    std::string title_pad = " " + title + " ";
    size_t title_len = title_pad.length();

    size_t dash_total = box_width - title_len;
    size_t dash_left  = dash_total / 2;
    size_t dash_right = dash_total - dash_left;

    // Title bar
    scf::print(Globals::g_THEME_COLOR, "\n┌", RESET);
    printDash(dash_left);
    scf::print(BOLD, title_pad, RESET);
    printDash(dash_right);
    scf::print(Globals::g_THEME_COLOR, "┐\n", RESET);

    // Static items
    for (const auto& item : menuItems) {

        printMenuLine(false, item, inner_width);

    }

    // Bottom border
    std::cout << Globals::g_THEME_COLOR << "└";
    printDash(box_width);
    std::cout << Globals::g_THEME_COLOR << "┘\n" << RESET;

    // Move cursor up to first item
    std::cout << "\r\033[" << (total + 1) << "A";

    // Selection loop
    while (true) {

        for (int i = 0; i < total; i++) {

            std::cout << "\r";
            printMenuLine(i == selected, menuItems[i], inner_width);

        }

        std::cout << "\033[" << total << "A";

        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) continue;

        if (c == '\x1b') {

            char seq[2];

            if (read(STDIN_FILENO, &seq, 2) == 2) {

                if (seq[1] == 'A') selected = (selected - 1 + total) % total;
                if (seq[1] == 'B') selected = (selected + 1) % total;

            }

        }
        else if (c == '\n' || c == '\r') {
            break;
        }
    }

    // Move cursor below menu
    std::cout << "\r\033[" << (total + 1) << "B\n";

    term.restoreTerminal();
    return menuItems[selected].first;
}

