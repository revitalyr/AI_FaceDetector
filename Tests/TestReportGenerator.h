#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextDocument>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>

namespace Glance::Testing {

struct TestResult {
    QString testName;
    QString category;
    bool passed = false;
    qint64 durationMs = 0;
    QString errorMessage;
    QStringList warnings;
    QJsonObject metrics;
    QDateTime timestamp;
};

struct TestBenchmarkResult {
    QString testName;
    double value = 0.0;
    QString unit;
    QString category;
    QDateTime timestamp;
    QJsonObject details;
};

struct TestSuiteSummary {
    QString suiteName;
    QDateTime startTime;
    QDateTime endTime;
    int totalTests = 0;
    int passedTests = 0;
    int failedTests = 0;
    int skippedTests = 0;
    qint64 totalDurationMs = 0;
    double successRate = 0.0;
    QString environment;
    QJsonObject systemInfo;
};

class TestReportGenerator : public QObject
{

public:
    explicit TestReportGenerator(QObject *parent = nullptr);
    ~TestReportGenerator();

    // Report configuration
    void setOutputDirectory(const QString& directory);
    void setReportTitle(const QString& title);
    void setEnvironmentInfo(const QJsonObject& info);
    
    // Test result collection
    void addTestResult(const TestResult& result);
    void addBenchmarkResult(const TestBenchmarkResult& result);
    void startTestSuite(const QString& suiteName);
    void endTestSuite();
    
    // Report generation
    bool generateJsonReport();
    bool generateHtmlReport();
    bool generateXmlReport();
    bool generateMarkdownReport();
    bool generateCsvReport();
    bool generateAllReports();
    
    // Report analysis
    QJsonObject analyzeTestResults();
    QJsonObject compareWithBaseline(const QString& baselineFile);
    QStringList detectRegressions();
    QStringList detectPerformanceRegressions(double threshold = 0.1);
    
    // Utility functions
    QString generateSummary();
    QString generatePerformanceChart();
    QString generateTestCoverageReport();
    bool exportResults(const QString& format, const QString& filename);

private:
    // Report generation helpers
    QString generateHtmlHeader();
    QString generateHtmlFooter();
    QString generateHtmlSummary();
    QString generateHtmlTestResults();
    QString generateHtmlBenchmarkResults();
    QString generateHtmlCharts();
    QString generateHtmlAnalysis();
    
    QString generateMarkdownHeader();
    QString generateMarkdownSummary();
    QString generateMarkdownTestResults();
    QString generateMarkdownBenchmarkResults();
    
    void writeXmlTestSuite(QXmlStreamWriter& writer);
    void writeXmlTestCase(QXmlStreamWriter& writer, const TestResult& result);
    void writeXmlBenchmark(QXmlStreamWriter& writer, const TestBenchmarkResult& result);
    
    // Data analysis
    double calculateSuccessRate();
    QJsonObject calculateTestStatistics();
    QJsonObject calculateBenchmarkStatistics();
    QJsonObject generateTrendAnalysis();
    
    // Chart generation
    QString generatePerformanceTrendChart();
    QString generateTestResultPieChart();
    QString generateTimelineChart();
    
    // File operations
    bool writeToFile(const QString& filename, const QString& content);
    QString getReportFilePath(const QString& extension);
    
    // Data members
    QString m_outputDirectory;
    QString m_reportTitle;
    QJsonObject m_environmentInfo;
    
    QList<TestResult> m_testResults;
    QList<TestBenchmarkResult> m_benchmarkResults;
    TestSuiteSummary m_currentSuite;
    
    QDateTime m_suiteStartTime;
    bool m_suiteActive = false;
    
    // Report templates
    static const QString HTML_TEMPLATE;
    static const QString CSS_STYLES;
    static const QString JAVASCRIPT_CHARTS;
};

} // namespace Glance::Testing
