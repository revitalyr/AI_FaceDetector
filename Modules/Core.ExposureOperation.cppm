/**
 * @file Core.ExposureOperation.cppm
 * @brief Exposure adjustment as an undoable edit operation
 *
 * Concrete EditOperation that adjusts image exposure. Stores the
 * exposure value to enable both apply and undo of the adjustment.
 */
export module Core.ExposureOperation;

export import Core.EditOperation;

export namespace Glance::Core {

/** @brief Undoable exposure adjustment operation */
class ExposureOperation : public EditOperation {
public:
    explicit ExposureOperation(float exposure);

    void apply(QImage& image) const override;
    void undo(QImage& image) const override;
    QString description() const override;

private:
    float m_exposure;
};

}
