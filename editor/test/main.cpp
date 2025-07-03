#include "blacksite_editor/EditorApplication.h"

int main() {
    BlacksiteEditor::EditorApplication editor;

    if (!editor.Initialize()) {
        return -1;
    }

    editor.Run();

    return 0;
}
