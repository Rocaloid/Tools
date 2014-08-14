# Some of these codes are modified from Coursera/ml-006.

function theta = randtheta(insize, outsize)
    theta = zeros(outsize, 1 + insize);
    epsilon = 0.12;
    theta = rand(outsize, 1 + insize) * 2 * epsilon - epsilon;
end

load sample.dat

input_size  = columns(X);
hidden_size = 100;
output_size = 50;

lambda      = 5;
options     = optimset("MaxIter", 2000);

theta1 = randtheta(input_size, hidden_size);
theta2 = randtheta(hidden_size, output_size);
theta  = [theta1(:); theta2(:)];

Xtest = X(28:end, :);
ytest = y(28:end);

X = X(1:27, :);
y = y(1:27);

costfn = @(p) ann_cost(p, input_size, hidden_size, output_size, X, y', lambda);
[theta, cost] = fmincg(costfn, theta, options);

theta1 = reshape(theta(1:hidden_size * (input_size + 1)), ...
                 hidden_size, (input_size + 1));

theta2 = reshape(theta((1 + (hidden_size * (input_size + 1))):end), ...
                 output_size, (hidden_size + 1));


ann_predict(theta1, theta2, X)' - y

ytest
ann_predict(theta1, theta2, Xtest)'

