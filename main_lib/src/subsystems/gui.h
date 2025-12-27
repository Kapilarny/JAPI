//
// Created by kapil on 1.12.2025.
//

#ifndef JAPI_PRELOAD_GUI_H
#define JAPI_PRELOAD_GUI_H
#include <memory>
#include <functional>
#include <vector>

class gui_manager {
public:
    static void init();
    static gui_manager& get();

    void update();
    void register_tab_item(const std::string& item_name, const std::function<void()>& func);
private:
    gui_manager();

    void init_native_hooks(); // Defined in kiero/kiero_impl.cpp

    static inline std::unique_ptr<gui_manager> instance = nullptr;
    std::vector<std::pair<std::string,std::function<void()>>> registered_tab_items;
};

#endif //JAPI_PRELOAD_GUI_H