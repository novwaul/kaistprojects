import tensorflow as tf

iterator = tf.data.TextLineDataset(["data-02.csv"]).batch(10).make_one_shot_iterator()

dataset = iterator.get_next()

record_defaults = [[0.] for row in range(9)]
lines = tf.decode_csv(dataset, record_defaults= record_defaults)

inputX = tf.stack(lines[0:-1], axis= 1)
inputY = tf.stack(lines[-1:], axis= 1)

X = tf.placeholder(tf.float32, shape= [None, 8])
Y = tf.placeholder(tf.float32, shape= [None, 1])

W = tf.Variable(tf.random_normal([8, 1]))
b = tf.Variable(tf.random_normal([1]))

hypothesis = tf.sigmoid(tf.matmul(X, W) + b)
cost = -tf.reduce_mean(Y*tf.log(hypothesis) + (1 - Y)*tf.log(1 - hypothesis))
train = tf.train.GradientDescentOptimizer(learning_rate= 0.01).minimize(cost)

predicated = tf.cast(hypothesis > 0.5, dtype= tf.float32)
accuracy = tf.reduce_mean(tf.cast(tf.equal(predicated, Y), dtype= tf.float32))

with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    sampleX, sampleY = sess.run([inputX, inputY])
    for step in range(10001):
        sess.run(train, feed_dict= {X : sampleX, Y : sampleY})
        if step % 200 == 0:
            print(step, sess.run(cost, feed_dict= {X : sampleX, Y : sampleY}))
    h,c,a = sess.run([hypothesis, predicated, accuracy], feed_dict= {X : sampleX, Y : sampleY})
    print("\nHypothesis: ", h, "\nCorrect (Y): ", c, "\nAccuracy: ", a)
