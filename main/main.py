# main.ipynb or main.py

import sys
import os
import numpy as np
from sklearn.decomposition import TruncatedSVD

# Add the path to your compiled PSO library
lib_path = os.path.abspath(os.path.join("..", "libs", "build"))
if lib_path not in sys.path:
    sys.path.append(lib_path)

# Import the PSO module
import pso_nmf

# ------------------------------
# Example usage
# ------------------------------

# Random data X (features x samples)
g, k, s = 500, 2, 40
X = np.random.rand(g, s)

# Use SVD to initialize W (g x k)
svd = TruncatedSVD(n_components=k, n_iter=7, random_state=42)
W_init = svd.fit_transform(X)  # shape (g x k)
# Ensure non-negativity
W_init[W_init < 0] = 0.0

print("Initialized W using SVD:\n", W_init)

# Create PSO object
pso = pso_nmf.PSO()
pso.set_population(20)
pso.set_max_iters(1000)
pso.set_verbose(True)

# Fit PSO to optimize H given W_init and X
H = pso.fit(W_init, X)

print("H (optimized by PSO):\n", H)

# Optional: compute reconstruction error
reconstruction = W_init @ H
error = np.linalg.norm(X - reconstruction, 'fro')
print(f"Frobenius norm of reconstruction error: {error:.6f}")
