#include "ui/MenuIO.hpp"

static const menu_header_padding_t calcHeaderPadding(scf::str32 header_name_, bool no_color_mode, size_t free_space_) {

    const size_t header_name_len = header_name_.length();
    const size_t version_len = Globals::version.length();
    const size_t free_space = free_space_; // free white space between left border and right border; defautl is 46

    const size_t total_header_len = header_name_len + version_len;
    const float left_padding = (free_space_ - total_header_len) / 2;
    const float right_padding = (free_space_ - left_padding) - total_header_len + (no_color_mode ? 4 : 0); 
  
    scf::str32 sleft_padding(left_padding, ' ');
    scf::str32 sright_padding(right_padding, ' ');
    scf::str64 header_content = header_name_ + " " + Globals::version; 

    return {sleft_padding, sright_padding, header_content};
}

uint32_t MainMenuIO::colorTuiMenu(const std::vector<std::pair<MenuOptionsMain, std::string>> &menuItems) {
    term.enableRawMode();

    int selected = 0;
    int total = (int)menuItems.size();

    scf::str32 header_name = header_names[0];

    if (Globals::g_debug == true) { header_name = header_names[1]; }
    else if (Globals::stand_alone == true) { header_name = header_names[2]; }

    const menu_header_padding_t header_padding = calcHeaderPadding(header_name, false);

    scf::print_flush("\033[2J\033[H");
    scf::println("Use Up/Down arrows and Enter to select an option.\n");
    scf::println(Globals::g_THEME_COLOR, "┌─────────────────────────────────────────────────┐", RESET);
    scf::println(Globals::g_THEME_COLOR, "│ ", RESET, BOLD, header_padding.sleft_padding_, header_padding.header_content_, header_padding.sright_padding_, RESET, Globals::g_THEME_COLOR, " │", RESET);
    scf::println(Globals::g_THEME_COLOR, "├─────────────────────────────────────────────────┤", RESET);
    for (size_t i = 0; i < menuItems.size(); ++i) {

        scf::print(Globals::g_THEME_COLOR, "│ ", RESET);

        // Build inner content with fixed width
        std::ostringstream inner;
        inner << std::setw(2) << menuItems[i].first << ". " << std::left << std::setw(43) << menuItems[i].second;
        std::string innerStr = inner.str();

        if (menuItems[i].first == 0) {
            innerStr = scf::to_std_str(Globals::g_THEME_COLOR) + innerStr + RESET;
        }

        // Print right border and newline
        scf::print(Globals::g_THEME_COLOR, " │\n", RESET);

    }
    scf::print(Globals::g_THEME_COLOR, "└─────────────────────────────────────────────────┘\n", RESET);

    printf("\033[%dA", (total + 1));

    while (true) {

        for (int i = 0; i < total; i++) {
            printf("\r"); 

            // Build inner content
            std::ostringstream inner;
            inner << std::setw(2) << menuItems[i].first << ". "
                << std::left << std::setw(43) << menuItems[i].second;

            std::string innerStr = inner.str();

            if (menuItems[i].first == 0) {
                innerStr = scf::to_std_str(Globals::g_THEME_COLOR) + innerStr + RESET;
            }

            scf::print(Globals::g_THEME_COLOR, "│ ", RESET);

            if (i == selected) scf::print(INVERSE);
            scf::print(innerStr);
            if (i == selected) scf::print(RESET);

            scf::println(Globals::g_THEME_COLOR, " │", RESET);
        }

        // Move cursor back up to top of menu
        printf("\033[%dA", total);

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
    printf("\033[%dB\n", (total + 1));

    term.restoreTerminal();
    return selected;
}

uint32_t MainMenuIO::noColorTuiMenu(const std::vector<std::pair<MenuOptionsMain, std::string>> &menuItems) {
    term.enableRawMode();

    int selected = 0;
    int total = (int)menuItems.size();

    scf::str32 header_name = header_names[0];

    if (Globals::g_debug == true) { header_name = header_names[1]; }
    else if (Globals::stand_alone == true) { header_name = header_names[2]; }

    const menu_header_padding_t header_padding = calcHeaderPadding(header_name, true);

    scf::print_flush("\033[2J\033[H");
    scf::println("Use Up/Down arrows and Enter to select an option.\n");
    scf::println("┌─────────────────────────────────────────────────────┐");
    scf::println("│ ", BOLD, header_padding.sleft_padding_, header_padding.header_content_, header_padding.sright_padding_, RESET, " │");
    scf::println("├─────────────────────────────────────────────────────┤");

    for (size_t i = 0; i < menuItems.size(); ++i) {

        scf::print("│ ");

        // Build inner content with fixed width
        std::ostringstream inner;
        inner << std::setw(2) << menuItems[i].first << ". "
            << std::left << std::setw(44) << menuItems[i].second;

        scf::print(inner.str());

        scf::println("  │");
    }

    scf::println("└─────────────────────────────────────────────────────┘");

    // Move cursor UP to where the first selectable line is
    printf("\033[%dA", (total + 1));

    while (true) {
        // Redraw selector arrows
        for (int i = 0; i < total; i++) {
            printf("\r"); // go to start of line

            if (i == selected) scf::print("│ ", BOLD, "> ");
            else scf::print("│   ");

            std::ostringstream inner;
            inner << std::setw(2) << menuItems[i].first << ". "
                    << std::left << std::setw(44) << menuItems[i].second;

            scf::println(inner.str(), "  │", RESET);
        }

        // Move cursor back up to top of menu
        printf("\033[%dA", total);

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
    printf("\033[%dB\n", (total + 1));

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

uint32_t GenericMenuIO::noColorTuiMenu(const scf::str32 &title, const std::vector<std::pair<int, std::string>> &menuItems) {
    term.enableRawMode();

    int selected = 0;
    int total = menuItems.size();

    // Width calculations
    size_t inner_width = computeInnerWidth(title, menuItems);
    size_t box_width   = inner_width + 4;

    // Title with padding
    scf::str<34> title_pad = " " + title + " ";
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
    scf::print(Globals::g_THEME_COLOR, "└");
    printDash(box_width);
    scf::print(Globals::g_THEME_COLOR, "┘\n", RESET);

    // Move cursor up to first item
    printf("\r\033[%dA", (total + 1));

    // Selection loop
    while (true) {

        for (int i = 0; i < total; i++) {

            printf("\r");
            printMenuLine(i == selected, menuItems[i], inner_width);

        }

        printf("\033[%dA", total);

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
    printf("\r\033[%dB\n", (total + 1));

    term.restoreTerminal();
    return menuItems[selected].first;
}

