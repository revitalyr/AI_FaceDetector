module;
#include <QDebug>
module Core.EditStack;

namespace Glance::Core {

EditStack::EditStack() = default;
EditStack::~EditStack() = default;

void EditStack::push(std::unique_ptr<EditOperation> operation)
{
    // Remove any redo operations
    if (m_currentIndex < static_cast<int>(m_operations.size()) - 1) {
        m_operations.erase(m_operations.begin() + m_currentIndex + 1, m_operations.end());
    }

    m_operations.push_back(std::move(operation));
    m_currentIndex = static_cast<int>(m_operations.size()) - 1;
}

void EditStack::undo()
{
    if (canUndo()) {
        m_currentIndex--;
    }
}

void EditStack::redo()
{
    if (canRedo()) {
        m_currentIndex++;
    }
}

void EditStack::clear()
{
    m_operations.clear();
    m_currentIndex = -1;
}

bool EditStack::canUndo() const
{
    return m_currentIndex >= 0;
}

bool EditStack::canRedo() const
{
    return m_currentIndex < static_cast<int>(m_operations.size()) - 1;
}

int EditStack::size() const
{
    return static_cast<int>(m_operations.size());
}

void EditStack::applyAll(QImage& image) const
{
    for (int i = 0; i <= m_currentIndex; ++i) {
        m_operations[i]->apply(image);
    }
}

QString EditStack::currentDescription() const
{
    if (m_currentIndex >= 0 && m_currentIndex < static_cast<int>(m_operations.size())) {
        return m_operations[m_currentIndex]->description();
    }
    return "No operations";
}

} // namespace Glance::Core
