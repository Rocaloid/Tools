function g = sigmoidgrad(z)
    g = zeros(size(z));
    s = 1 ./ (1 + e .^ (- z));
    g = s .* (1 - s);
end

