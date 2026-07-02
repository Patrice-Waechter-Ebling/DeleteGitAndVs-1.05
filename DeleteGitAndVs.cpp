#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

using namespace std;
const char* Extensions[] ={"nuget.targets","build.csdef","appx","appxbundle","appxupload","dbmdl","dbproj.schemaview","jfm","pfx","publishsettings","snk","rptproj.bak","mdf","ldf","ndf","rdl.data","bim.layout","bim_*.settings","rptproj.rsuser","GhostDoc.xml","plg","opt","vbw","ncb","aps","pyc","tss","jmconfig","btp.cs","btm.cs","odx.cs","xsd.cs","binlog","nvuser","vsix","cab","msi","msix","msm","msp","tmp","suo","old","log","bak"};
const int nbrExtension = sizeof(Extensions) / sizeof(Extensions[0]);
const char* Dossiers[] ={".git",".vs","node_modules","Package","bin","obj","backup",".github",".vscode",".history",".idea",".builds"};
const int nbDossier = sizeof(Dossiers) / sizeof(Dossiers[0]);
static int nbTotalDossier = 0;
static int nbDossiersSupprimes = 0;
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";
const string BG_RED = "\033[41m";
const string BG_GREEN = "\033[42m";
const string BG_YELLOW = "\033[43m";
const string BG_BLUE = "\033[44m";
const string BG_MAGENTA = "\033[45m";
const string BG_CYAN = "\033[46m";
const string BG_WHITE = "\033[47m";

static void AfficherProgression()
{
    const int lngBarre = 50;
    float ratio = 0.0f;
    if (nbTotalDossier > 0)
        ratio = (float)nbDossiersSupprimes / (float)nbTotalDossier;
    int filled = (int)(ratio * lngBarre);
    printf("\r[");
    for (int i = 0; i < filled; i++) printf("#");
    for (int j = filled; j < lngBarre; j++) printf(" ");
    printf("] %d / %d", nbDossiersSupprimes, nbTotalDossier);
    fflush(stdout);
}
static bool IsTargetExtension(const char* name)
{
    const char* ext = strrchr(name, '.');
    if (!ext) return false;
    ext++;
    for (int i = 0; i < nbrExtension; i++)
    {
        if (!lstrcmpiA(ext, Extensions[i])) return true;
    }
    return false;
}
static bool DossierCibleValide(const char* name)
{
    for (int i = 0; i < nbDossier; i++)
    {
        if (!lstrcmpiA(name, Dossiers[i]))return true;
    }
    return false;
}
static bool SuppressionDossierRecursive(const char* path)
{
    char chemin[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE h;
    wsprintfA(chemin, "%s\\*.*", path);
    h = FindFirstFileA(chemin, &fd);
    if (h == INVALID_HANDLE_VALUE)return FALSE;
    do
    {
        if (!lstrcmpA(fd.cFileName, ".") || !lstrcmpA(fd.cFileName, ".."))continue;
        char fullPath[MAX_PATH];
        wsprintfA(fullPath, "%s\\%s", path, fd.cFileName);
        SetFileAttributesA(fullPath, FILE_ATTRIBUTE_NORMAL);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            SuppressionDossierRecursive(fullPath);
            RemoveDirectoryA(fullPath);
        }
        else{DeleteFileA(fullPath);}
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
    return RemoveDirectoryA(path);
}
static void CompteCible(const char* root)
{
    char chemin[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE h;
    wsprintfA(chemin, "%s\\*.*", root);
    h = FindFirstFileA(chemin, &fd);
    if (h == INVALID_HANDLE_VALUE)return;
    do
    {
        if (!lstrcmpA(fd.cFileName, ".") || !lstrcmpA(fd.cFileName, ".."))continue;
        char fullPath[MAX_PATH];
        wsprintfA(fullPath, "%s\\%s", root, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (DossierCibleValide(fd.cFileName)){nbTotalDossier++;}
            else{CompteCible(fullPath);}
        }

    } while (FindNextFileA(h, &fd));
    FindClose(h);
}
struct StyleCadre {
    string tl, tr, bl, br;  // coins
    string h, v;            // horizontal, vertical
    string color;           // couleur ANSI
};
const StyleCadre Simple = { "+", "+", "+", "+", "-", "|", CYAN };
const StyleCadre Double = { "É", "»", "È", "¼", "Í", "º", YELLOW };
const StyleCadre Epais = { "Û", "Û", "Û", "Û", "Û", "Û", MAGENTA };
static int RetourneLargeur() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}
static int RetourneHauteur() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
#endif
}
static string CentrerTexte(const string& text, size_t width) {
    if (text.size() >= width) return text.substr(0, width);
    size_t padding = (width - text.size()) / 2;
    size_t extra = (width - text.size()) % 2;
    return string(padding, ' ') + text + string(padding + extra, ' ');
}
static vector<string> GenererCadre(const vector<string>& lines,
    const StyleCadre& style,
    bool center,
    bool background) {
    int consoleWidth = RetourneLargeur();
    int consoleHeight = RetourneHauteur();
    size_t maxLen = 0;
    for (const auto& line : lines)maxLen = max(maxLen, line.size());
    if (maxLen + 4 > (size_t)consoleWidth)maxLen = consoleWidth - 4;
    size_t innerWidth = maxLen;
    vector<string> frame;
    frame.push_back(style.color + style.tl + string(innerWidth, style.h[0]) + style.tr + RESET);
    for (const auto& line : lines) {
        string content = line;
        if (content.size() > maxLen)content = content.substr(0, maxLen);
        if (center)
            content = CentrerTexte(content, maxLen);
        else
            content += string(maxLen - content.size(), ' ');
        string row = style.color + style.v + RESET + (background ? BG_BLUE : "") + content + RESET + style.color + style.v + RESET;
        frame.push_back(row);
    }
    frame.push_back(style.color + style.bl + string(innerWidth, style.h[0]) + style.br + RESET);
    return frame;
}
static void AfficherCadre(const vector<string>& frame) {
    for (const auto& line : frame) cout << line << "\n";
}
static void Ombrager(const vector<string>& frame) {
    for (const auto& line : frame)cout << line << " ø" << "\n";
    cout << string(frame[0].size(), ' ') << " øøø" << "\n";
}
static void SuppressionRecursive(const char* root)
{
    char chemin[MAX_PATH];
    WIN32_FIND_DATAA fd;
    HANDLE h;
    wsprintfA(chemin, "%s\\*.*", root);
    h = FindFirstFileA(chemin, &fd);
    if (h == INVALID_HANDLE_VALUE)return;
    do
    {
        if (!lstrcmpA(fd.cFileName, ".") || !lstrcmpA(fd.cFileName, ".."))continue;
        char fullPath[MAX_PATH];
        wsprintfA(fullPath, "%s\\%s", root, fd.cFileName);
        SetFileAttributesA(fullPath, FILE_ATTRIBUTE_NORMAL);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (DossierCibleValide(fd.cFileName))
            {
                printf("\nSuppression dossier : %s\n", fullPath);
                SuppressionDossierRecursive(fullPath);
                nbDossiersSupprimes++;
                AfficherProgression();
            }
            else{SuppressionRecursive(fullPath);}
        }
        else
        {
            if (IsTargetExtension(fd.cFileName) ||(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                printf("Fichier supprime : %s\n", fullPath);
                DeleteFileA(fullPath);
            }
        }

    } while (FindNextFileA(h, &fd));
    FindClose(h);
}
static bool ExplorerDossiers(char* outPath)
{
    BROWSEINFOA bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.lpszTitle = "Selectionnez le dossier … nettoyer";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN|BIF_STATUSTEXT|BIF_EDITBOX|BIF_VALIDATE|BIF_BROWSEFORCOMPUTER;
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (!pidl)return false;
    BOOL ok = SHGetPathFromIDListA(pidl, outPath);
    CoTaskMemFree(pidl);
    return ok ? true : false;
}
int main()
{
    char path[MAX_PATH];
    vector<string> lines;
    string input;
    const StyleCadre* style = &Double;
    lines.push_back("");
    lines.push_back(" Cr‚ation Patrice Waechter-Ebling ");
    lines.push_back("");
    lines.push_back(" Bienvenue dans l'outil de nettoyage pour dossiers! ");
    lines.push_back(" de codes sources pour projets VisualStudio et GIT. ");
    lines.push_back("");
    AfficherCadre(GenererCadre(lines, *style, true, true)); // Centrage activ‚, fond activ‚
    lines.clear();    if (!ExplorerDossiers(path)){printf("Aucun dossier selectionn‚.\n");return 0;}
    printf("\nDossier selectionn‚ : %s\n", path);
    printf("\nAnalyse des dossiers...\n");
    CompteCible(path);
    printf("Dossiers … supprimer : %d\n", nbTotalDossier);
    printf("Nettoyage en cours...\n");
    AfficherProgression();
    SuppressionRecursive(path);
    printf("\n\nTermine.\n");
    return 0;
}
