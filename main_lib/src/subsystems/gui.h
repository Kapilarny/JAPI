//
// Created by kapil on 1.12.2025.
//

#ifndef JAPI_PRELOAD_GUI_H
#define JAPI_PRELOAD_GUI_H
#include <memory>

class gui_manager {
public:
    static void init();
    static gui_manager& get();

    void update();
private:
    gui_manager();

    static inline std::unique_ptr<gui_manager> instance = nullptr;

    void init_native_hooks(); // Defined in kiero/kiero_impl.cpp
};

#endif //JAPI_PRELOAD_GUI_H