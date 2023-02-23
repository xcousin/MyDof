@echo off
gcc dof_guide_win.c -o dof_guide_win.exe -lopenbgi -lgdi32 -lcomdlg32 -luuid -loleaut32 -lole32 -std=c99 -lm -g