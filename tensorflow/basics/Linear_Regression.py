import tensorflow as tf

iterator = tf.data.TextLineDataset(["data-01-test-score.csv.txt"])\
                                    .skip(1)\
                                    .repeat()\
                                    .batch(10)\
                                    .make_initializable_iterator()
dataset = iterator.get_next()

record_defaults = [[0.], [0.], [0.], [0.]]
lines = tf.decode_csv(dataset, record_defaults= record_defaults)

train_x_batch = tf.stack(lines[0:-1], axis = 1)
train_y_batch = tf.stack(lines[-1:], axis = 1)

X = tf.placeholder(tf.float32, shape= [None, 3])
Y = tf.placeholder(tf.float32, shape= [None, 1])

W = tf.Variable(tf.random_normal([3, 1]))
b = tf.Variable(tf.random_normal([1]))

H = tf.matmul(X,W) + b
cost = tf.reduce_mean(tf.square(H - Y))

opt = tf.train.GradientDescentOptimizer(learning_rate= 1e-5)
train = opt.minimize(cost)

sess = tf.Session()
sess.run(tf.global_variables_initializer())

for step in range(2001):
    sess.run(iterator.initializer)
    x_batch, y_batch = sess.run([train_x_batch, train_y_batch])
    loss_val, h_val, _ = sess.run([cost, H, train], feed_dict={X: x_batch, Y: y_batch})
    if step % 10 == 0:
        print("loss : {0}".format(loss_val))

print("score..", sess.run(H, feed_dict={X:[[100,70,101]]}))
