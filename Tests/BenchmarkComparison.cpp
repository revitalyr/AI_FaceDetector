#include "BenchmarkComparison.h"
#include <QStandardPaths>
#include <QTextStream>
#include <QJsonArray>
#include <QDebug>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QtMath>

namespace Glance::Testing {

// Static definitions
const QString BenchmarkComparison::BASELINE_FILE_NAME = "benchmark_baseline.json";
const QString BenchmarkComparison::RESULTS_FILE_NAME = "benchmark_results.json";
const QString BenchmarkComparison::ALERTS_FILE_NAME = "benchmark_alerts.json";
const QString BenchmarkComparison::REPORT_FILE_NAME = "benchmark_report.html";

BenchmarkComparison::BenchmarkComparison(QObject *parent)
    : QObject(parent)
    , m_defaultThreshold(0.1) // 10% default threshold
{
    // Set default working directory
    m_workingDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/GlanceBenchmarks";
    ensureDirectoryExists(m_workingDirectory);
    
    // Collect environment information
    m_environmentInfo["qt_version"] = QT_VERSION_STR;
    m_environmentInfo["os"] = QSysInfo::prettyProductName();
    m_environmentInfo["architecture"] = QSysInfo::currentCpuArchitecture();
    m_environmentInfo["hostname"] = QSysInfo::machineHostName();
    
    // Load existing data
    loadBaseline(getBaselineFilePath());
    loadResults(getResultsFilePath());
}

BenchmarkComparison::~BenchmarkComparison() = default;

bool BenchmarkComparison::loadBaseline(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open baseline file:" << filePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        qDebug() << "Invalid baseline file format";
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray baselines = root["baselines"].toArray();
    
    m_baselines.clear();
    
    for (const auto& value : baselines) {
        QJsonObject baselineObj = value.toObject();
        
        BenchmarkBaseline baseline;
        baseline.testName = baselineObj["test_name"].toString();
        baseline.baselineValue = baselineObj["baseline_value"].toDouble();
        baseline.threshold = baselineObj["threshold"].toDouble();
        baseline.baselineDate = QDateTime::fromString(baselineObj["baseline_date"].toString(), Qt::ISODate);
        baseline.environment = baselineObj["environment"].toString();
        baseline.metadata = baselineObj["metadata"].toObject();
        
        m_baselines[baseline.testName] = baseline;
    }
    
    qDebug() << QString("Loaded %1 baseline entries").arg(m_baselines.size());
    return true;
}

bool BenchmarkComparison::saveBaseline(const QString& filePath)
{
    QJsonObject root;
    QJsonArray baselines;
    
    for (auto it = m_baselines.begin(); it != m_baselines.end(); ++it) {
        const BenchmarkBaseline& baseline = it.value();
        
        QJsonObject baselineObj;
        baselineObj["test_name"] = baseline.testName;
        baselineObj["baseline_value"] = baseline.baselineValue;
        baselineObj["threshold"] = baseline.threshold;
        baselineObj["baseline_date"] = baseline.baselineDate.toString(Qt::ISODate);
        baselineObj["environment"] = baseline.environment;
        baselineObj["metadata"] = baseline.metadata;
        
        baselines.append(baselineObj);
    }
    
    root["baselines"] = baselines;
    root["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["environment"] = m_environmentInfo;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open baseline file for writing:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

bool BenchmarkComparison::addBaseline(const QString& testName, double baselineValue, double threshold)
{
    if (testName.isEmpty()) {
        return false;
    }
    
    BenchmarkBaseline baseline;
    baseline.testName = testName;
    baseline.baselineValue = baselineValue;
    baseline.threshold = threshold > 0 ? threshold : m_defaultThreshold;
    baseline.baselineDate = QDateTime::currentDateTime();
    baseline.environment = m_environmentInfo["os"].toString();
    
    m_baselines[testName] = baseline;
    
    baselineUpdated(testName, baselineValue);
    return true;
}

bool BenchmarkComparison::removeBaseline(const QString& testName)
{
    return m_baselines.remove(testName) > 0;
}

void BenchmarkComparison::clearBaselines()
{
    m_baselines.clear();
}

void BenchmarkComparison::addBenchmarkResult(const BenchmarkResult& result)
{
    if (validateBenchmarkResult(result)) {
        m_results.append(result);
        
        // Check for regression immediately
        if (isRegression(result.testName, result.value)) {
            RegressionAlert alert;
            alert.testName = result.testName;
            alert.currentValue = result.value;
            alert.alertType = "regression";
            m_alerts.append(alert);
            regressionDetected(alert);
        }
    }
}

void BenchmarkComparison::addBenchmarkResults(const QList<BenchmarkResult>& results)
{
    for (const BenchmarkResult& result : results) {
        addBenchmarkResult(result);
    }
}

bool BenchmarkComparison::loadResults(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open results file:" << filePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        qDebug() << "Invalid results file format";
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray results = root["results"].toArray();
    
    m_results.clear();
    
    for (const auto& value : results) {
        QJsonObject resultObj = value.toObject();
        
        BenchmarkResult result;
        result.testName = resultObj["test_name"].toString();
        result.category = resultObj["category"].toString();
        result.value = resultObj["value"].toDouble();
        result.unit = resultObj["unit"].toString();
        result.timestamp = QDateTime::fromString(resultObj["timestamp"].toString(), Qt::ISODate);
        result.metadata = resultObj["metadata"].toObject();
        result.environment = resultObj["environment"].toString();
        result.gitCommit = resultObj["git_commit"].toString();
        result.qtVersion = resultObj["qt_version"].toString();
        result.osVersion = resultObj["os_version"].toString();
        
        if (validateBenchmarkResult(result)) {
            m_results.append(result);
        }
    }
    
    qDebug() << QString("Loaded %1 benchmark results").arg(m_results.size());
    return true;
}

bool BenchmarkComparison::saveResults(const QString& filePath)
{
    QJsonObject root;
    QJsonArray results;
    
    for (const BenchmarkResult& result : m_results) {
        QJsonObject resultObj;
        resultObj["test_name"] = result.testName;
        resultObj["category"] = result.category;
        resultObj["value"] = result.value;
        resultObj["unit"] = result.unit;
        resultObj["timestamp"] = result.timestamp.toString(Qt::ISODate);
        resultObj["metadata"] = result.metadata;
        resultObj["environment"] = result.environment;
        resultObj["git_commit"] = result.gitCommit;
        resultObj["qt_version"] = result.qtVersion;
        resultObj["os_version"] = result.osVersion;
        
        results.append(resultObj);
    }
    
    root["results"] = results;
    root["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["environment"] = m_environmentInfo;
    
    QJsonDocument doc(root);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open results file for writing:" << filePath;
        return false;
    }
    
    file.write(doc.toJson());
    return true;
}

void BenchmarkComparison::clearResults()
{
    m_results.clear();
}

QList<RegressionAlert> BenchmarkComparison::detectRegressions()
{
    m_alerts.clear();
    
    // Group results by test name
    QMap<QString, QList<BenchmarkResult>> resultsByTest;
    for (const BenchmarkResult& result : m_results) {
        resultsByTest[result.testName].append(result);
    }
    
    // Check each test for regressions
    for (auto it = resultsByTest.begin(); it != resultsByTest.end(); ++it) {
        const QString& testName = it.key();
        const QList<BenchmarkResult>& testResults = it.value();
        
        if (testResults.isEmpty()) {
            continue;
        }
        
        // Get the most recent result
        BenchmarkResult latestResult = testResults.last();
        
        if (isRegression(testName, latestResult.value)) {
            RegressionAlert alert;
            alert.testName = latestResult.testName;
            alert.currentValue = latestResult.value;
            alert.alertType = "regression";
            m_alerts.append(alert);
        }
    }
    
    return m_alerts;
}

QList<RegressionAlert> BenchmarkComparison::detectRegressionsForTest(const QString& testName)
{
    QList<RegressionAlert> testAlerts;
    
    // Find results for this test
    QList<BenchmarkResult> testResults;
    for (const BenchmarkResult& result : m_results) {
        if (result.testName == testName) {
            testResults.append(result);
        }
    }
    
    if (testResults.isEmpty()) {
        return testAlerts;
    }
    
    // Check the most recent result
    BenchmarkResult latestResult = testResults.last();
    
    if (isRegression(testName, latestResult.value)) {
        RegressionAlert alert;
        alert.testName = testName;
        alert.currentValue = latestResult.value;
        alert.alertType = "regression";
        testAlerts.append(alert);
    }
    
    return testAlerts;
}

bool BenchmarkComparison::isRegression(const QString& testName, double currentValue)
{
    if (!m_baselines.contains(testName)) {
        return false; // No baseline to compare against
    }
    
    const BenchmarkBaseline& baseline = m_baselines[testName];
    double regressionPercent = calculateRegressionPercent(testName, currentValue);
    
    return regressionPercent > baseline.threshold;
}

double BenchmarkComparison::calculateRegressionPercent(const QString& testName, double currentValue)
{
    if (!m_baselines.contains(testName)) {
        return 0.0;
    }
    
    const BenchmarkBaseline& baseline = m_baselines[testName];
    
    if (baseline.baselineValue == 0.0) {
        return 0.0; // Avoid division by zero
    }
    
    return ((currentValue - baseline.baselineValue) / baseline.baselineValue) * 100.0;
}

QMap<QString, TrendData> BenchmarkComparison::analyzeTrends(int windowSize)
{
    QMap<QString, TrendData> trends;
    
    // Group results by test name
    QMap<QString, QList<BenchmarkResult>> resultsByTest;
    for (const BenchmarkResult& result : m_results) {
        resultsByTest[result.testName].append(result);
    }
    
    // Analyze trends for each test
    for (auto it = resultsByTest.begin(); it != resultsByTest.end(); ++it) {
        const QString& testName = it.key();
        const QList<BenchmarkResult>& testResults = it.value();
        
        if (testResults.size() < 2) {
            continue; // Need at least 2 points for trend analysis
        }
        
        TrendData trendData;
        trendData.testName = testName;
        
        // Create data points
        for (const BenchmarkResult& result : testResults) {
            trendData.dataPoints.append(qMakePair(result.timestamp, result.value));
        }
        
        // Sort by timestamp
        std::sort(trendData.dataPoints.begin(), trendData.dataPoints.end(),
                 [](const QPair<QDateTime, double>& a, const QPair<QDateTime, double>& b) {
                     return a.first < b.first;
                 });
        
        // Calculate moving average
        QVector<double> values;
        for (const auto& point : trendData.dataPoints) {
            values.append(point.second);
        }
        
        trendData.movingAverage = calculateMovingAverage(values, windowSize);
        
        // Calculate slope (trend)
        trendData.slope = calculateSlope(trendData.dataPoints);
        
        // Determine trend direction
        if (qAbs(trendData.slope) < 0.01) {
            trendData.trend = "stable";
        } else if (trendData.slope > 0) {
            trendData.trend = "degrading";
        } else {
            trendData.trend = "improving";
        }
        
        trends[testName] = trendData;
    }
    
    trendAnalysisCompleted(trends);
    return trends;
}

TrendData BenchmarkComparison::getTrendData(const QString& testName, int windowSize)
{
    QMap<QString, TrendData> allTrends = analyzeTrends(windowSize);
    return allTrends.value(testName, TrendData());
}

QString BenchmarkComparison::getTrendDirection(const QString& testName)
{
    TrendData trend = getTrendData(testName);
    return trend.trend;
}

QJsonObject BenchmarkComparison::generatePerformanceReport()
{
    QJsonObject report;
    
    // Summary statistics
    report["summary"] = getSummaryStatistics();
    
    // Regression alerts
    QJsonArray alerts;
    for (const RegressionAlert& alert : m_alerts) {
        QJsonObject alertObj;
        alertObj["test_name"] = alert.testName;
        alertObj["current_value"] = alert.currentValue;
        alertObj["baseline_value"] = alert.baselineValue;
        alertObj["regression_percent"] = alert.regressionPercent;
        alertObj["severity"] = alert.severity;
        alertObj["description"] = alert.description;
        alertObj["detected"] = alert.detected.toString(Qt::ISODate);
        alertObj["acknowledged"] = alert.acknowledged;
        alerts.append(alertObj);
    }
    report["regressions"] = alerts;
    
    // Trend analysis
    QMap<QString, TrendData> trends = analyzeTrends();
    QJsonObject trendsObj;
    for (auto it = trends.begin(); it != trends.end(); ++it) {
        const TrendData& trend = it.value();
        
        QJsonObject trendObj;
        trendObj["test_name"] = trend.testName;
        trendObj["moving_average"] = trend.movingAverage;
        trendObj["trend"] = trend.trend;
        trendObj["slope"] = trend.slope;
        trendObj["data_points"] = trend.dataPoints.size();
        
        trendsObj[it.key()] = trendObj;
    }
    report["trends"] = trendsObj;
    
    // Performance statistics
    QJsonObject perfStats;
    QMap<QString, double> stats = getPerformanceStats();
    for (auto it = stats.begin(); it != stats.end(); ++it) {
        perfStats[it.key()] = it.value();
    }
    report["performance_stats"] = perfStats;
    
    // Baseline statistics
    QJsonObject baselineStats;
    QMap<QString, double> baselineStatsMap = getBaselineStats();
    for (auto it = baselineStatsMap.begin(); it != baselineStatsMap.end(); ++it) {
        baselineStats[it.key()] = it.value();
    }
    report["baseline_stats"] = baselineStats;
    
    report["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    report["environment"] = m_environmentInfo;
    
    performanceReportGenerated(report);
    return report;
}

QJsonObject BenchmarkComparison::compareWithBaseline(const QString& baselineFile)
{
    QJsonObject comparison;
    
    // Load external baseline
    BenchmarkComparison externalComparator;
    if (!externalComparator.loadBaseline(baselineFile)) {
        comparison["error"] = "Failed to load external baseline file";
        return comparison;
    }
    
    // Compare baselines
    QJsonArray comparisons;
    
    for (auto it = m_baselines.begin(); it != m_baselines.end(); ++it) {
        const QString& testName = it.key();
        const BenchmarkBaseline& currentBaseline = it.value();
        
        if (externalComparator.m_baselines.contains(testName)) {
            const BenchmarkBaseline& externalBaseline = externalComparator.m_baselines[testName];
            
            QJsonObject comparisonObj;
            comparisonObj["test_name"] = testName;
            comparisonObj["current_baseline"] = currentBaseline.baselineValue;
            comparisonObj["external_baseline"] = externalBaseline.baselineValue;
            comparisonObj["difference"] = currentBaseline.baselineValue - externalBaseline.baselineValue;
            comparisonObj["difference_percent"] = ((currentBaseline.baselineValue - externalBaseline.baselineValue) / externalBaseline.baselineValue) * 100.0;
            comparisonObj["current_date"] = currentBaseline.baselineDate.toString(Qt::ISODate);
            comparisonObj["external_date"] = externalBaseline.baselineDate.toString(Qt::ISODate);
            
            comparisons.append(comparisonObj);
        }
    }
    
    comparison["comparisons"] = comparisons;
    comparison["generated_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return comparison;
}

QJsonObject BenchmarkComparison::generateComparisonReport(const QString& baselineFile)
{
    QJsonObject report = compareWithBaseline(baselineFile);
    
    // Add additional analysis
    if (report.contains("comparisons")) {
        QJsonArray comparisons = report["comparisons"].toArray();
        
        int totalComparisons = comparisons.size();
        int improvedCount = 0;
        int degradedCount = 0;
        int unchangedCount = 0;
        
        for (const auto& value : comparisons) {
            QJsonObject comparison = value.toObject();
            double diffPercent = comparison["difference_percent"].toDouble();
            
            if (qAbs(diffPercent) < 1.0) {
                unchangedCount++;
            } else if (diffPercent > 0) {
                degradedCount++;
            } else {
                improvedCount++;
            }
        }
        
        QJsonObject summary;
        summary["total_comparisons"] = totalComparisons;
        summary["improved"] = improvedCount;
        summary["degraded"] = degradedCount;
        summary["unchanged"] = unchangedCount;
        summary["improvement_rate"] = totalComparisons > 0 ? (improvedCount * 100.0) / totalComparisons : 0.0;
        summary["degradation_rate"] = totalComparisons > 0 ? (degradedCount * 100.0) / totalComparisons : 0.0;
        
        report["summary"] = summary;
    }
    
    return report;
}

QMap<QString, double> BenchmarkComparison::getPerformanceStats()
{
    QMap<QString, double> stats;
    
    if (m_results.isEmpty()) {
        return stats;
    }
    
    // Group results by test name
    QMap<QString, QList<double>> valuesByTest;
    for (const BenchmarkResult& result : m_results) {
        valuesByTest[result.testName].append(result.value);
    }
    
    // Calculate statistics for each test
    for (auto it = valuesByTest.begin(); it != valuesByTest.end(); ++it) {
        const QString& testName = it.key();
        const QList<double>& values = it.value();
        
        if (values.isEmpty()) {
            continue;
        }
        
        // Calculate mean
        double sum = 0.0;
        for (double value : values) {
            sum += value;
        }
        double mean = sum / values.size();
        
        // Calculate standard deviation
        double variance = 0.0;
        for (double value : values) {
            variance += qPow(value - mean, 2);
        }
        double stdDev = qSqrt(variance / values.size());
        
        // Find min and max
        double minVal = *std::min_element(values.begin(), values.end());
        double maxVal = *std::max_element(values.begin(), values.end());
        
        stats[QString("%1_mean").arg(testName)] = mean;
        stats[QString("%1_stddev").arg(testName)] = stdDev;
        stats[QString("%1_min").arg(testName)] = minVal;
        stats[QString("%1_max").arg(testName)] = maxVal;
        stats[QString("%1_samples").arg(testName)] = values.size();
    }
    
    return stats;
}

QMap<QString, double> BenchmarkComparison::getBaselineStats()
{
    QMap<QString, double> stats;
    
    if (m_baselines.isEmpty()) {
        return stats;
    }
    
    QList<double> baselineValues;
    for (auto it = m_baselines.begin(); it != m_baselines.end(); ++it) {
        baselineValues.append(it.value().baselineValue);
    }
    
    if (baselineValues.isEmpty()) {
        return stats;
    }
    
    // Calculate statistics
    double sum = 0.0;
    for (double value : baselineValues) {
        sum += value;
    }
    double mean = sum / baselineValues.size();
    
    double variance = 0.0;
    for (double value : baselineValues) {
        variance += qPow(value - mean, 2);
    }
    double stdDev = qSqrt(variance / baselineValues.size());
    
    double minVal = *std::min_element(baselineValues.begin(), baselineValues.end());
    double maxVal = *std::max_element(baselineValues.begin(), baselineValues.end());
    
    stats["baseline_mean"] = mean;
    stats["baseline_stddev"] = stdDev;
    stats["baseline_min"] = minVal;
    stats["baseline_max"] = maxVal;
    stats["baseline_count"] = baselineValues.size();
    
    return stats;
}

QJsonObject BenchmarkComparison::getSummaryStatistics()
{
    QJsonObject summary;
    
    summary["total_baselines"] = m_baselines.size();
    summary["total_results"] = m_results.size();
    summary["total_regressions"] = m_alerts.size();
    summary["unacknowledged_regressions"] = getUnacknowledgedRegressions().size();
    
    // Calculate regression statistics
    int lowSeverity = 0, mediumSeverity = 0, highSeverity = 0, criticalSeverity = 0;
    for (const RegressionAlert& alert : m_alerts) {
        if (alert.severity == "low") lowSeverity++;
        else if (alert.severity == "medium") mediumSeverity++;
        else if (alert.severity == "high") highSeverity++;
        else if (alert.severity == "critical") criticalSeverity++;
    }
    
    summary["regressions_low"] = lowSeverity;
    summary["regressions_medium"] = mediumSeverity;
    summary["regressions_high"] = highSeverity;
    summary["regressions_critical"] = criticalSeverity;
    
    summary["default_threshold"] = m_defaultThreshold;
    summary["working_directory"] = m_workingDirectory;
    
    return summary;
}

void BenchmarkComparison::acknowledgeRegression(const QString& testName)
{
    for (RegressionAlert& alert : m_alerts) {
        if (alert.testName == testName) {
            alert.acknowledged = true;
        }
    }
}

QList<RegressionAlert> BenchmarkComparison::getUnacknowledgedRegressions()
{
    QList<RegressionAlert> unacknowledged;
    
    for (const RegressionAlert& alert : m_alerts) {
        if (!alert.acknowledged) {
            unacknowledged.append(alert);
        }
    }
    
    return unacknowledged;
}

void BenchmarkComparison::setRegressionThreshold(const QString& testName, double threshold)
{
    if (m_baselines.contains(testName)) {
        m_baselines[testName].threshold = threshold;
    }
}

bool BenchmarkComparison::exportToCSV(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "Test Name,Category,Value,Unit,Timestamp,Environment,Git Commit,Qt Version,OS Version\n";
    
    // Write data
    for (const BenchmarkResult& result : m_results) {
        out << result.testName << ","
            << result.category << ","
            << result.value << ","
            << result.unit << ","
            << result.timestamp.toString(Qt::ISODate) << ","
            << result.environment << ","
            << result.gitCommit << ","
            << result.qtVersion << ","
            << result.osVersion << "\n";
    }
    
    return true;
}

bool BenchmarkComparison::exportToJSON(const QString& filePath)
{
    return saveResults(filePath);
}

bool BenchmarkComparison::exportBaselineToJSON(const QString& filePath)
{
    return saveBaseline(filePath);
}

QString BenchmarkComparison::generateHTMLReport()
{
    QString html;
    QTextStream stream(&html);
    
    // HTML header
    stream << R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Benchmark Performance Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        .container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; border-bottom: 2px solid #007bff; padding-bottom: 10px; }
        h2 { color: #666; margin-top: 30px; }
        table { width: 100%; border-collapse: collapse; margin: 20px 0; }
        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }
        th { background-color: #f8f9fa; font-weight: bold; }
        .regression { background-color: #f8d7da; }
        .improvement { background-color: #d4edda; }
        .stable { background-color: #fff3cd; }
        .severity-low { border-left: 4px solid #ffc107; }
        .severity-medium { border-left: 4px solid #fd7e14; }
        .severity-high { border-left: 4px solid #dc3545; }
        .severity-critical { border-left: 4px solid #6f42c1; }
        .summary-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin: 20px 0; }
        .summary-card { background-color: #f8f9fa; padding: 15px; border-radius: 8px; text-align: center; }
        .summary-card h3 { margin: 0 0 10px 0; color: #666; }
        .summary-card .value { font-size: 2em; font-weight: bold; color: #333; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Benchmark Performance Report</h1>
        <p>Generated: )" << QDateTime::currentDateTime().toString() << R"(</p>
)";
    
    // Summary section
    QJsonObject summary = getSummaryStatistics();
    stream << "<div class=\"summary-grid\">";
    
    stream << "<div class=\"summary-card\">";
    stream << "<h3>Total Baselines</h3>";
    stream << "<div class=\"value\">" << summary["total_baselines"].toInt() << "</div>";
    stream << "</div>";
    
    stream << "<div class=\"summary-card\">";
    stream << "<h3>Total Results</h3>";
    stream << "<div class=\"value\">" << summary["total_results"].toInt() << "</div>";
    stream << "</div>";
    
    stream << "<div class=\"summary-card\">";
    stream << "<h3>Regressions</h3>";
    stream << "<div class=\"value\">" << summary["total_regressions"].toInt() << "</div>";
    stream << "</div>";
    
    stream << "<div class=\"summary-card\">";
    stream << "<h3>Unacknowledged</h3>";
    stream << "<div class=\"value\">" << summary["unacknowledged_regressions"].toInt() << "</div>";
    stream << "</div>";
    
    stream << "</div>";
    
    // Regressions section
    if (!m_alerts.isEmpty()) {
        stream << "<h2>Regressions Detected</h2>";
        stream << "<table>";
        stream << "<tr><th>Test Name</th><th>Current Value</th><th>Baseline Value</th><th>Regression %</th><th>Severity</th><th>Description</th></tr>";
        
        for (const RegressionAlert& alert : m_alerts) {
            QString severityClass = QString("severity-%1").arg(alert.severity);
            
            stream << "<tr class=\"" << severityClass << "\">";
            stream << "<td>" << alert.testName << "</td>";
            stream << "<td>" << alert.currentValue << "</td>";
            stream << "<td>" << alert.baselineValue << "</td>";
            stream << "<td>" << QString::number(alert.regressionPercent, 'f', 2) << "%</td>";
            stream << "<td>" << alert.severity << "</td>";
            stream << "<td>" << alert.description << "</td>";
            stream << "</tr>";
        }
        
        stream << "</table>";
    }
    
    // Performance statistics
    QMap<QString, double> perfStats = getPerformanceStats();
    if (!perfStats.isEmpty()) {
        stream << "<h2>Performance Statistics</h2>";
        stream << "<table>";
        stream << "<tr><th>Test</th><th>Mean</th><th>Std Dev</th><th>Min</th><th>Max</th><th>Samples</th></tr>";
        
        // Group by test name
        QMap<QString, QList<QString>> testNames;
        for (auto it = perfStats.begin(); it != perfStats.end(); ++it) {
            QString key = it.key();
            if (key.endsWith("_mean")) {
                QString testName = key.left(key.length() - 5);
                testNames[testName].append(key);
            }
        }
        
        for (auto it = testNames.begin(); it != testNames.end(); ++it) {
            const QString& testName = it.key();
            
            stream << "<tr>";
            stream << "<td>" << testName << "</td>";
            stream << "<td>" << perfStats[QString("%1_mean").arg(testName)] << "</td>";
            stream << "<td>" << perfStats[QString("%1_stddev").arg(testName)] << "</td>";
            stream << "<td>" << perfStats[QString("%1_min").arg(testName)] << "</td>";
            stream << "<td>" << perfStats[QString("%1_max").arg(testName)] << "</td>";
            stream << "<td>" << perfStats[QString("%1_samples").arg(testName)] << "</td>";
            stream << "</tr>";
        }
        
        stream << "</table>";
    }
    
    // Footer
    stream << R"(
        <footer>
            <p>Generated by Glance Benchmark Comparison System</p>
        </footer>
    </div>
</body>
</html>
)";
    
    return html;
}

QString BenchmarkComparison::generateMarkdownReport()
{
    QString markdown;
    QTextStream stream(&markdown);
    
    stream << "# Benchmark Performance Report\n\n";
    stream << "**Generated:** " << QDateTime::currentDateTime().toString() << "\n\n";
    
    // Summary
    QJsonObject summary = getSummaryStatistics();
    stream << "## Summary\n\n";
    stream << "| Metric | Value |\n";
    stream << "|--------|-------|\n";
    stream << "| Total Baselines | " << summary["total_baselines"].toInt() << " |\n";
    stream << "| Total Results | " << summary["total_results"].toInt() << " |\n";
    stream << "| Regressions | " << summary["total_regressions"].toInt() << " |\n";
    stream << "| Unacknowledged | " << summary["unacknowledged_regressions"].toInt() << " |\n\n";
    
    // Regressions
    if (!m_alerts.isEmpty()) {
        stream << "## Regressions Detected\n\n";
        stream << "| Test Name | Current | Baseline | Regression % | Severity | Description |\n";
        stream << "|-----------|---------|----------|--------------|----------|-------------|\n";
        
        for (const RegressionAlert& alert : m_alerts) {
            stream << "| " << alert.testName
                   << " | " << alert.currentValue
                   << " | " << alert.baselineValue
                   << " | " << QString::number(alert.regressionPercent, 'f', 2) << "%"
                   << " | " << alert.severity
                   << " | " << alert.description << " |\n";
        }
        stream << "\n";
    }
    
    // Performance statistics
    QMap<QString, double> perfStats = getPerformanceStats();
    if (!perfStats.isEmpty()) {
        stream << "## Performance Statistics\n\n";
        stream << "| Test | Mean | Std Dev | Min | Max | Samples |\n";
        stream << "|------|------|---------|-----|-----|---------|\n";
        
        // Group by test name
        QMap<QString, QList<QString>> testNames;
        for (auto it = perfStats.begin(); it != perfStats.end(); ++it) {
            QString key = it.key();
            if (key.endsWith("_mean")) {
                QString testName = key.left(key.length() - 5);
                testNames[testName].append(key);
            }
        }
        
        for (auto it = testNames.begin(); it != testNames.end(); ++it) {
            const QString& testName = it.key();
            
            stream << "| " << testName
                   << " | " << perfStats[QString("%1_mean").arg(testName)]
                   << " | " << perfStats[QString("%1_stddev").arg(testName)]
                   << " | " << perfStats[QString("%1_min").arg(testName)]
                   << " | " << perfStats[QString("%1_max").arg(testName)]
                   << " | " << perfStats[QString("%1_samples").arg(testName)] << " |\n";
        }
    }
    
    stream << "\n---\n";
    stream << "*Generated by Glance Benchmark Comparison System*\n";
    
    return markdown;
}

void BenchmarkComparison::setDefaultThreshold(double threshold)
{
    m_defaultThreshold = qBound(0.0, threshold, 1.0);
}

void BenchmarkComparison::setWorkingDirectory(const QString& directory)
{
    m_workingDirectory = directory;
    ensureDirectoryExists(m_workingDirectory);
}

void BenchmarkComparison::setEnvironmentInfo(const QJsonObject& info)
{
    m_environmentInfo = info;
}

// Private helper methods

double BenchmarkComparison::calculateMovingAverage(const QVector<double>& values, int windowSize)
{
    if (values.isEmpty()) {
        return 0.0;
    }
    
    int actualWindowSize = qMin(windowSize, values.size());
    double sum = 0.0;
    
    // Calculate average of the last 'windowSize' values
    for (int i = values.size() - actualWindowSize; i < values.size(); ++i) {
        sum += values[i];
    }
    
    return sum / actualWindowSize;
}

double BenchmarkComparison::calculateSlope(const QVector<QPair<QDateTime, double>>& dataPoints)
{
    if (dataPoints.size() < 2) {
        return 0.0;
    }
    
    // Simple linear regression to calculate slope
    double n = dataPoints.size();
    double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
    
    for (int i = 0; i < dataPoints.size(); ++i) {
        double x = i; // Use index as x coordinate
        double y = dataPoints[i].second;
        
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }
    
    return (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
}

QString BenchmarkComparison::determineSeverity(double regressionPercent)
{
    if (regressionPercent < 5.0) {
        return "low";
    } else if (regressionPercent < 15.0) {
        return "medium";
    } else if (regressionPercent < 30.0) {
        return "high";
    } else {
        return "critical";
    }
}

QString BenchmarkComparison::generateRegressionDescription(const RegressionAlert& alert)
{
    return QString("Performance regression detected in %1: %2% slower than baseline (%3 vs %4)")
           .arg(alert.testName)
           .arg(QString::number(alert.regressionPercent, 'f', 1))
           .arg(alert.currentValue)
           .arg(alert.baselineValue);
}

bool BenchmarkComparison::ensureDirectoryExists(const QString& path)
{
    QDir dir(path);
    return dir.mkpath(path);
}

QString BenchmarkComparison::getBaselineFilePath()
{
    return m_workingDirectory + "/" + BASELINE_FILE_NAME;
}

QString BenchmarkComparison::getResultsFilePath()
{
    return m_workingDirectory + "/" + RESULTS_FILE_NAME;
}

QString BenchmarkComparison::getAlertsFilePath()
{
    return m_workingDirectory + "/" + ALERTS_FILE_NAME;
}

bool BenchmarkComparison::validateBenchmarkResult(const BenchmarkResult& result)
{
    return !result.testName.isEmpty() && result.value >= 0.0;
}

bool BenchmarkComparison::validateBaseline(const BenchmarkBaseline& baseline)
{
    return !baseline.testName.isEmpty() && baseline.baselineValue >= 0.0;
}

} // namespace Glance::Testing
