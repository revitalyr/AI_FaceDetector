module;

#include <vector>
#include <optional>
#include <cstdint>

/**
 * @file Core.LUT.cppm
 * @brief Cube LUT (Look-Up Table) for color grading
 *
 * Provides CubeLUT struct for loading and applying 3D colour lookup
 * tables in the Adobe Cube format, commonly used for professional
 * colour grading and colour correction.
 */
export module Core.LUT;

import Qt.Wrapper;

export
{
    using ::QString;
}

export namespace Glance::Core {

/**
 * @brief Cube LUT (Look-Up Table) for color grading
 * 
 * This structure represents a 3D color lookup table in the Adobe Cube format,
 * commonly used for professional color grading and color correction.
 * 
 * The LUT maps input RGB values to output RGB values through a 3D grid,
 * allowing for sophisticated color transformations.
 * 
 * Typical LUT sizes are 32x32x32, 33x33x33, or 64x64x64.
 */
struct CubeLUT {
    int size = 0;                    ///< Size of the 3D LUT grid (e.g., 32 for 32x32x32)
    std::vector<float> data;         ///< Flat array of LUT data (size * size * size * 3 floats)

    /**
     * @brief Get pointer to the LUT data
     * @return Pointer to the float array
     */
    float* table() { return data.data(); }
    
    /**
     * @brief Get const pointer to the LUT data
     * @return Const pointer to the float array
     */
    const float* table() const { return data.data(); }
    
    /**
     * @brief Calculate the total size of the LUT data
     * @return Total number of floats in the data array
     */
    size_t totalSize() const { return static_cast<size_t>(size) * size * size * 3; }
    
    /**
     * @brief Check if the LUT is valid
     * @return True if size > 0 and data size matches expected size
     */
    bool isValid() const { return size > 0 && data.size() == totalSize(); }

    /**
     * @brief Parse a Cube LUT file from disk
     * @param filePath Path to the .cube file
     * @return Optional containing the parsed LUT, or nullopt on failure
     */
    static std::optional<CubeLUT> parse(const QString& filePath);

    /**
     * @brief Apply the LUT to RGB color values
     * @param r Red component (input/output, modified in place)
     * @param g Green component (input/output, modified in place)
     * @param b Blue component (input/output, modified in place)
     * 
     * This performs trilinear interpolation in the 3D LUT grid to transform
     * the input color to the output color.
     */
    void apply(float& r, float& g, float& b) const;
};

}
