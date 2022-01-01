#ifndef SEARCHWINDOW_H
#define SEARCHWINDOW_H

#include <QMainWindow>
#include "../stemminglib/english_stem.h"
#include <filesystem>
#include <chrono>
#include <sstream>
#include <QPixmap>
#include <QAbstractItemView>
#include <QListView>
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QStringList>
#include <QDesktopServices>

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class searchWindow; }
QT_END_NAMESPACE

class searchWindow : public QMainWindow
{
    Q_OBJECT

public:
    searchWindow(QWidget *parent = nullptr);
    ~searchWindow();

private slots:
    void on_resultsList_doubleClicked(const QModelIndex &index);

    void on_toolButton_clicked();

private:
    Ui::searchWindow *ui;
    unordered_map<wstring, vector<wstring>> docs_info;
    vector<unordered_map<wstring, int>> lineNums;
    unordered_map<wstring, int> lexicon;
    vector<wstring> getAndParseQuery(wstring query);
    unordered_map<wstring, int> getLexicon();
    unordered_map<wstring, vector<wstring>> getDocsInfo();
    vector<unordered_map<wstring, int>> getLineNumbers();
    void customIntersection(vector<vector<pair<wstring,wstring>>> &);
    void makeCombiUtil(vector<vector<int>>&, vector<int>&, int, int, int);
    vector<vector<int> > makeCombi(int, int);
    double getCumScore(const vector<int> &queryWordIDs, unordered_map<int, pair<wstring, int>> &posWordMap, const vector<int> &hitPos);
    QStringListModel* resultListModel;
    QStringList* results;
};
#endif // SEARCHWINDOW_H
