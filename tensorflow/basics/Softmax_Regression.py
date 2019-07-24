import tensorflow as tf

iterator = tf.data.TextLineDataset(["data-03.txt"]).batch(3).make_initializable_iterator()

dataset = iterator.get_next()

record_defaults = [[0.] for row in range(6)]
lines = tf.decode_csv(dataset, record_defaults= record_defaults)

inputX = tf.stack(lines[0:-1], axis= 1)
inputY = tf.stack(lines[-1:], axis= 1)

X = tf.placeholder(tf.float32, shape= [None, 5])
Y = tf.placeholder(tf.int32, shape= [None, 1])

Y_onehot = tf.one_hot(Y, 3)
Y_onehot = tf.reshape(Y_onehot, [-1, 3])

W = tf.Variable(tf.random_normal([5, 3]))
b = tf.Variable(tf.random_normal([3]))

logits = tf.matmul(X, W) + b
hypothesis = tf.nn.softmax(logits)
cost_i = tf.nn.softmax_cross_entropy_with_logits(logits=logits, labels=Y_onehot)
cost = tf.reduce_mean(cost_i)
train = tf.train.GradientDescentOptimizer(learning_rate= 0.01).minimize(cost)

predicated = tf.argmax(hypothesis, 1)
accuracy = tf.reduce_mean(tf.cast(tf.equal(predicated, tf.argmax(Y_onehot, 1)), dtype= tf.float32))

with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    for step in range(10001):
        sess.run(iterator.initializer)
        try:
            while True:
                sampleX, sampleY = sess.run([inputX, inputY])
                sess.run(train, feed_dict= {X : sampleX, Y : sampleY})
        except tf.errors.OutOfRangeError:
            pass
        if step % 200 == 0:
            print(step, sess.run(cost, feed_dict= {X : sampleX, Y : sampleY}))
    sess.run(iterator.initializer)
    try:
        while True:
            sampleX, sampleY = sess.run([inputX, inputY])
            h,c,a, y = sess.run([hypothesis, predicated, accuracy, Y], feed_dict= {X : sampleX, Y : sampleY})
            print("\nHypothesis: ", h, "\nPrediction: ", c, "\nActual: ", y, "\nAccuracy: ", a)
    except tf.errors.OutOfRangeError:
        pass
