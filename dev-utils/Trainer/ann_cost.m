function [J grad] = ann_cost(nn_params, ...
    input_size, hidden_size, output_size, X, y, lambda)

    Theta1 = reshape(nn_params(1:hidden_size * (input_size + 1)), ...
                     hidden_size, (input_size + 1));

    Theta2 = reshape(nn_params((1 + (hidden_size * (input_size + 1))):end), ...
                     output_size, (hidden_size + 1));

    m = size(X, 1);
    J = 0;
    Theta1_grad = zeros(size(Theta1));
    Theta2_grad = zeros(size(Theta2));

    a1 = [ones(1, m); X'];
    z2 = Theta1 * a1;
    a2 = [ones(1, m); sigmoid(z2)];
    z3 = Theta2 * a2;
    h  = sigmoid(z3);
    
    y2 = zeros(m, rows(h));
    g  = gaussian(200, 0.2);
    for i = 1:m
        y2(i, :) = g(101 - y(i):150 - y(i));
    end
    
    y2 = y2';
    J  = sum(sum(- (y2 .* log(h)) - (1 - y2) .* log(1 - h))) / m;
    J += lambda / 2 / m ...
       * (sum(sum(Theta1(1:rows(Theta1), 2:columns(Theta1)) .^ 2)) ...
       +  sum(sum(Theta2(1:rows(h), 2:columns(Theta2)) .^ 2)));
    
    d3 = h - y2;
    
    col2 = columns(Theta2);
    col1 = columns(Theta1);
    row2 = rows(Theta2);
    row1 = rows(Theta1);
    
    D1 = zeros(row1, col1);
    D2 = zeros(row2, col2);
    for t = 1:m
        d2  = (Theta2' * d3(:, t))(2:end) .* sigmoidgrad(z2(:, t));
        D1 += d2 * a1(:, t)';
        D2 += d3(:, t) * a2(:, t)';
    end
    
    Theta1_grad = (D1 + lambda * [zeros(row1, 1), Theta1(:, 2:end)]) / m;
    Theta2_grad = (D2 + lambda * [zeros(row2, 1), Theta2(:, 2:end)]) / m;
    
    grad = [Theta1_grad(:) ; Theta2_grad(:)];
end



