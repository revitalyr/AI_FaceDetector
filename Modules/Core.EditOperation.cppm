/**
 * @file Core.EditOperation.cppm
 * @brief Abstract base class for undoable image edit operations
 *
 * Defines the EditOperation interface that all concrete edit operations
 * (e.g., ExposureOperation) must implement. Supports apply/undo semantics
 * for the EditStack undo/redo system.
 */
module;

#include <memory>
#include <vector>

export module Core.EditOperation;

import Qt.Wrapper;

export
{
    using ::QImage;
    using ::QString;
}

export namespace Glance::Core {

/** @brief Abstract interface for undoable image edit operations */
class EditOperation {
public:
    virtual ~EditOperation() = default;

    virtual void apply(QImage& image) const = 0;
    virtual void undo(QImage& image) const = 0;
    virtual QString description() const = 0;
};

}
