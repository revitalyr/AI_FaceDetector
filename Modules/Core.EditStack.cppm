module;

#include <memory>
#include <vector>

/**
 * @file Core.EditStack.cppm
 * @brief Undo/redo stack for image editing operations
 *
 * Manages a stack of EditOperation instances, allowing users to undo
 * and redo changes to images. Maintains a current position in the
 * stack and can apply all operations up to that point.
 */
export module Core.EditStack;

import Qt.Wrapper;
export import Core.EditOperation;

export
{
    using ::QImage;
    using ::QString;
}

export namespace Glance::Core {

/**
 * @brief Undo/redo stack for image editing operations
 * 
 * This class manages a stack of edit operations, allowing users to undo and
 * redo changes to images. It maintains a current position in the stack and
 * can apply all operations up to that point.
 * 
 * Typical usage:
 * - Push operations onto the stack as edits are made
 * - Call undo() to revert the last operation
 * - Call redo() to reapply the last undone operation
 * - Call applyAll() to apply all operations to an image
 */
class EditStack {
public:
    /**
     * @brief Construct an empty edit stack
     */
    EditStack();
    
    /**
     * @brief Destructor - cleans up all operations
     */
    ~EditStack();

    /**
     * @brief Push a new edit operation onto the stack
     * @param operation Unique pointer to the edit operation to add
     * 
     * Any operations after the current position will be removed when
     * pushing a new operation (standard undo/redo behavior).
     */
    void push(std::unique_ptr<EditOperation> operation);
    
    /**
     * @brief Undo the last operation
     * 
     * Moves the current position back by one operation.
     */
    void undo();
    
    /**
     * @brief Redo the last undone operation
     * 
     * Moves the current position forward by one operation.
     */
    void redo();
    
    /**
     * @brief Clear all operations from the stack
     */
    void clear();

    /**
     * @brief Check if undo is possible
     * @return True if there are operations to undo, false otherwise
     */
    bool canUndo() const;
    
    /**
     * @brief Check if redo is possible
     * @return True if there are operations to redo, false otherwise
     */
    bool canRedo() const;
    
    /**
     * @brief Get the total number of operations in the stack
     * @return Total operation count
     */
    int size() const;

    /**
     * @brief Apply all operations up to the current position to an image
     * @param image Reference to the image to apply operations to
     * 
     * This applies all operations from the beginning of the stack up to
     * the current position in sequence.
     */
    void applyAll(QImage& image) const;

    /**
     * @brief Get a description of the current operation
     * @return Description string, or empty if no current operation
     */
    QString currentDescription() const;

private:
    std::vector<std::unique_ptr<EditOperation>> m_operations;
    int m_currentIndex = -1;
};

}
