#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QVector>
#include <QPair>

namespace Glance::Testing {

struct BenchmarkResult {
    QString testName;
    QString category;
    double value = 0.0;
    QString unit;
    QDateTime timestamp;
    QJsonObject metadata;
    QString environment;
    QString gitCommit;
    QString qtVersion;
    QString osVersion;
};

struct BenchmarkBaseline {
    QString testName;
    double baselineValue = 0.0;
    double threshold = 0.1; // 10% threshold by default
    QDateTime baselineDate;
    QString environment;
    QJsonObject metadata;
};

struct RegressionAlert {
    QString testName;
    QString alertType;
    double currentValue = 0.0;
    double baselineValue = 0.0;
    double regressionPercent = 0.0;
    double threshold = 0.0;
    QString severity; // "low", "medium", "high", "critical"
    QString description;
    QDateTime detected;
    bool acknowledged = false;
};

struct TrendData {
    QString testName;
    QVector<QPair<QDateTime, double>> dataPoints;
    double movingAverage = 0.0;
    QString trend; // "improving", "degrading", "stable"
    double slope = 0.0;
};

class BenchmarkComparison : public QObject
{

public:
    explicit BenchmarkComparison(QObject *parent = nullptr);
    ~BenchmarkComparison();

    // Baseline management
    bool loadBaseline(const QString& filePath);
    bool saveBaseline(const QString& filePath);
    bool addBaseline(const QString& testName, double baselineValue, double threshold = 0.1);
    bool removeBaseline(const QString& testName);
    void clearBaselines();
    
    // Result management
    void addBenchmarkResult(const BenchmarkResult& result);
    void addBenchmarkResults(const QList<BenchmarkResult>& results);
    bool loadResults(const QString& filePath);
    bool saveResults(const QString& filePath);
    void clearResults();
    
    // Comparison and regression detection
    QList<RegressionAlert> detectRegressions();
    QList<RegressionAlert> detectRegressionsForTest(const QString& testName);
    bool isRegression(const QString& testName, double currentValue);
    double calculateRegressionPercent(const QString& testName, double currentValue);
    
    // Trend analysis
    QMap<QString, TrendData> analyzeTrends(int windowSize = 10);
    TrendData getTrendData(const QString& testName, int windowSize = 10);
    QString getTrendDirection(const QString& testName);
    
    // Performance analysis
    QJsonObject generatePerformanceReport();
    QJsonObject compareWithBaseline(const QString& baselineFile);
    QJsonObject generateComparisonReport(const QString& baselineFile);
    
    // Statistics
    QMap<QString, double> getPerformanceStats();
    QMap<QString, double> getBaselineStats();
    QJsonObject getSummaryStatistics();
    
    // Alert management
    void acknowledgeRegression(const QString& testName);
    QList<RegressionAlert> getUnacknowledgedRegressions();
    void setRegressionThreshold(const QString& testName, double threshold);
    
    // Report generation
    QString generateReport();
    
    // Export and reporting
    bool exportToCSV(const QString& filePath);
    bool exportToJSON(const QString& filePath);
    bool exportBaselineToJSON(const QString& filePath);
    QString generateHTMLReport();
    QString generateMarkdownReport();
    
    // Configuration
    void setDefaultThreshold(double threshold);
    void setWorkingDirectory(const QString& directory);
    void setEnvironmentInfo(const QJsonObject& info);

private:
    // Data analysis helpers
    double calculateMovingAverage(const QVector<double>& values, int windowSize);
    double calculateSlope(const QVector<QPair<QDateTime, double>>& dataPoints);
    QString determineSeverity(double regressionPercent);
    QString generateRegressionDescription(const RegressionAlert& alert);
    
    // File operations
    bool ensureDirectoryExists(const QString& path);
    QString getBaselineFilePath();
    QString getResultsFilePath();
    QString getAlertsFilePath();
    
    // Validation
    bool validateBenchmarkResult(const BenchmarkResult& result);
    bool validateBaseline(const BenchmarkBaseline& baseline);
    
    // Data members
    QMap<QString, BenchmarkBaseline> m_baselines;
    QList<BenchmarkResult> m_results;
    QList<RegressionAlert> m_alerts;
    
    QString m_workingDirectory;
    double m_defaultThreshold;
    QJsonObject m_environmentInfo;
    
    // Configuration
    static const QString BASELINE_FILE_NAME;
    static const QString RESULTS_FILE_NAME;
    static const QString ALERTS_FILE_NAME;
    static const QString REPORT_FILE_NAME;
};

} // namespace Glance::Testing
