X = lattice4(3);
y = sample_from_matern(X, [1;1;1;1], 1);

tic;[xi1, iters1] = dl_bgfs(X, y, [1;2;3;4], 10, 0.2, 1e-6);toc
tic;[xi2, iters2] = dl_sr1(X, y, [1;2;3;4], 5, 0.1, 0.15, 1e-6);toc
tic;[xi3, iters3] = dl_id(X, y, [1;2;3;4], 5, 0.15, 1e-6);toc
tic;[xi4, iters4] = dl_hess(X, y, [1;2;3;4], 5, 0.1, 0.015, 1e-6);toc

tic;[xi5, iters5] = dl_bgfs(X, y, [2;4;8;16], 10, 0.2, 1e-6);toc
tic;[xi6, iters6] = dl_sr1(X, y, [2;4;8;16], 5, 0.1, 0.15, 1e-6);toc
tic;[xi7, iters7] = dl_id(X, y, [2;4;8;16], 5, 0.15, 1e-6);toc
tic;[xi8, iters8] = dl_hess(X, y, [2;4;8;16], 5, 0.1, 0.015, 1e-6);toc