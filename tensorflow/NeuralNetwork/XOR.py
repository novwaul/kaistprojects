import tensorflow as tf

x_data = [[0., 0.], [0., 1.], [1., 0.], [1., 1.]]
y_data = [[0.], [1.], [1.], [0.]]

X = tf.placeholder(tf.float32, shape= [None, 2])
Y = tf.placeholder(tf.float32, shape= [None, 1])

#layer1
with tf.name_scope("layer1") as scope:
    W1 = tf.Variable(tf.random_normal([2, 10]))
    b1 = tf.Variable(tf.random_normal([1, 10]))
    layer1 = tf.sigmoid(tf.matmul(X, W1) + b1)

#layer2
with tf.name_scope("layer2") as scope:
    W2 = tf.Variable(tf.random_normal([10, 1]))
    b2 = tf.Variable(tf.random_normal([1]))
    hypothesis = tf.sigmoid(tf.matmul(layer1, W2) + b2)

cost = -tf.reduce_mean(Y*tf.log(hypothesis) + (1-Y)*tf.log(1 - hypothesis))
train = tf.train.GradientDescentOptimizer(learning_rate= 0.01).minimize(cost)

#for TensorBorad
cost_summ = tf.summary.scalar("cost", cost)
summary = tf.summary.merge_all()

prediction = tf.cast(hypothesis > 0.5, dtype= tf.float32)
accuracy = tf.reduce_mean(tf.cast(tf.equal(prediction, Y), dtype= tf.float32))

with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    writer = tf.summary.FileWriter('./logs')
    writer.add_graph(sess.graph)
    for step in range(10001):
        s, _ = sess.run([summary, train], feed_dict= {X: x_data, Y: y_data})
        writer.add_summary(s, global_step= step)
        if step % 100 == 0:
            print(step, ": cost [", sess.run(cost, feed_dict= {X: x_data, Y: y_data}), "], accuracy [",sess.run(accuracy, feed_dict= {X: x_data, Y: y_data}), "]")
    x, y, _, p = sess.run([X, Y, train, prediction], feed_dict= {X: x_data, Y: y_data})
    print("Input: \n", x, "\npreciction: \n", p, "\nactual: \n", y)
