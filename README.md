### DeleteGitAndVs-1.05
#### bug résolu: 
+ Exception 0xC0000409 => détection d’un dépassement de tampon sur la pile (/GS failure).
+ Trace et code montrent des buffers locaux (chemin[MAX_PATH], fullPath[MAX_PATH]) remplis avec wsprintfA sans vérification de taille.
+ Si root ou fd.cFileName rendent la chaîne plus longue que MAX_PATH, buffer overflow et rupture du cookie de pile.
+ L’appel récursif CompteCible(fullPath) propage des chemins potentiellement trop longs, aggravant le problème.
##### Correction:
+ Utiliser StringCchPrintfA / StringCchCatA (strsafe.h) ou snprintf_s en vérifiant la taille.
+ Gérer les chemins dépassant MAX_PATH (préfixe \?\ ou APIs wide + PathCchCombine si besoin).
+  Remplacer récursion par itération si profondeur importante.
### Code original
    static void CompteCible(const char* root)
    {
        char chemin[MAX_PATH];
        WIN32_FIND_DATAA fd;
        HANDLE h;
        HRESULT hr = StringCchPrintfA(chemin, MAX_PATH, "%s\\*.*", root);
        if (FAILED(hr)) return; // chemin trop long ou erreur
        h = FindFirstFileA(chemin, &fd);
        if (h == INVALID_HANDLE_VALUE) return;
        do
        {
            if (!lstrcmpA(fd.cFileName, ".") || !lstrcmpA(fd.cFileName, "..")) continue;
            char fullPath[MAX_PATH];
            hr = StringCchPrintfA(fullPath, MAX_PATH, "%s\\%s", root, fd.cFileName);
            if (FAILED(hr)) continue; // ignorer entrée qui dépasserait la taille
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (DossierCibleValide(fd.cFileName)) { nbTotalDossier++; }
                else { CompteCible(fullPath); }
            }
    
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
  ### Code modifié
      static void CompteCible(const char* root)
      {
          const int BUF = 32768;
          wchar_t rootW[BUF];
          if (MultiByteToWideChar(CP_ACP, 0, root, -1, rootW, BUF) == 0) return;
          CompteCibleW(rootW);
      }
<img width="1366" height="720" alt="image" src="https://github.com/user-attachments/assets/14ea3699-3ea4-465d-959d-71960ad19043" />
